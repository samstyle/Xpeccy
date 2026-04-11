#include "emulwin.h"

#include <QRegularExpression>
#include <algorithm>
#include <vector>

#ifdef USEOPENGL

namespace {

// Interleaved quad: position (x,y) + texcoord (u,v), triangle strip order
// matching the legacy glBegin sequence (RT, LT, RB, LB). Positions are in
// [0,1]² and the vertex shader maps them to NDC with Y flipped so that the
// texture's top-left sample lands at the window's top-left.
constexpr GLfloat kQuadVertices[] = {
	1.0f, 0.0f,  1.0f, 0.0f,
	0.0f, 0.0f,  0.0f, 0.0f,
	1.0f, 1.0f,  1.0f, 1.0f,
	0.0f, 1.0f,  0.0f, 1.0f,
};

bool currentContextIsGLES() {
	QOpenGLContext* ctx = QOpenGLContext::currentContext();
	return ctx && ctx->isOpenGLES();
}

constexpr const char* glslVersionPrefix(bool es) {
	return es
		? "#version 300 es\nprecision highp float;\n"
		: "#version 330\n";
}

const char* defaultVertexShaderBody() {
	return
		"layout(location=0) in vec2 a_pos;\n"
		"layout(location=1) in vec2 a_uv;\n"
		"out vec2 v_uv;\n"
		"void main() {\n"
		"    v_uv = a_uv;\n"
		"    gl_Position = vec4(a_pos.x * 2.0 - 1.0, 1.0 - a_pos.y * 2.0, 0.0, 1.0);\n"
		"}\n";
}

const char* defaultFragmentShaderBody() {
	return
		"in vec2 v_uv;\n"
		"out vec4 fragColor;\n"
		"uniform sampler2D u_tex;\n"
		"void main() {\n"
		"    fragColor = texture(u_tex, v_uv);\n"
		"}\n";
}

QString composeShader(const char* body) {
	return QString::fromLatin1(glslVersionPrefix(currentContextIsGLES()))
	     + QString::fromLatin1(body);
}

// -----------------------------------------------------------------------------
// Legacy GLSL 110 shim.
//
// Rewrites fixed-function-era shader source onto the core-compatible VBO
// attribute and uniform layout. The transformation is entirely data-driven
// by the tables below — to add a new legacy identifier, append a row to the
// relevant table; to add a new preamble attribute or uniform, edit the stage
// preamble helper. No change to loadShader or the shim pipeline is needed.
// -----------------------------------------------------------------------------

struct Rewrite {
	QRegularExpression pattern;
	QString replacement;
};

// Build a word-boundary-bounded regex for an identifier. Lookaround is used
// instead of \b so tokens ending in punctuation (e.g. gl_TexCoord[0]) can
// still be exact-matched as a future extension.
QRegularExpression wordBoundedRegex(const char* identifier) {
	return QRegularExpression(
		QStringLiteral(R"((?<!\w))")
		+ QRegularExpression::escape(QLatin1String(identifier))
		+ QStringLiteral(R"((?!\w))"));
}

Rewrite rewrite(const char* identifier, const char* replacement) {
	return { wordBoundedRegex(identifier), QString::fromLatin1(replacement) };
}

// Pure: apply a list of rewrites via left fold.
QString applyRewrites(QString s, const std::vector<Rewrite>& rewrites) {
	for (const auto& r : rewrites) s.replace(r.pattern, r.replacement);
	return s;
}

// Signals whose presence in the source triggers the shim pipeline.
// Absence of all of them lets a modern shader pass through untouched.
const std::vector<QRegularExpression>& legacySignalPatterns() {
	static const std::vector<QRegularExpression> r{
		wordBoundedRegex("gl_Vertex"),
		wordBoundedRegex("gl_MultiTexCoord0"),
		wordBoundedRegex("gl_ModelViewProjectionMatrix"),
		wordBoundedRegex("gl_FragColor"),
		wordBoundedRegex("varying"),
	};
	return r;
}

bool sourceNeedsLegacyShim(const QString& src) {
	const auto& patterns = legacySignalPatterns();
	return std::any_of(patterns.begin(), patterns.end(),
		[&src](const QRegularExpression& re) { return src.contains(re); });
}

// Pure: strip any existing `#version` directive so the shim can supply its own.
QString stripVersionDirective(QString s) {
	static const QRegularExpression re(
		QStringLiteral(R"(^[ \t]*#[ \t]*version\b[^\n]*\n?)"),
		QRegularExpression::MultilineOption);
	s.replace(re, QString{});
	return s;
}

// Rewrites applied to both vertex and fragment stages.
const std::vector<Rewrite>& sharedRewrites() {
	static const std::vector<Rewrite> r{
		rewrite("texture2D", "texture"),
	};
	return r;
}

// Vertex-only rewrites.
const std::vector<Rewrite>& vertexRewrites() {
	static const std::vector<Rewrite> r{
		rewrite("varying",                      "out"),
		rewrite("gl_Vertex",                    "vec4(a_pos_.x, a_pos_.y, 0.0, 1.0)"),
		rewrite("gl_MultiTexCoord0",            "vec4(a_uv_.x,  a_uv_.y,  0.0, 0.0)"),
		rewrite("gl_ModelViewProjectionMatrix", "u_mvp_"),
	};
	return r;
}

// Fragment-only rewrites.
const std::vector<Rewrite>& fragmentRewrites() {
	static const std::vector<Rewrite> r{
		rewrite("varying",      "in"),
		rewrite("gl_FragColor", "frag_out_"),
	};
	return r;
}

QString vertexShimPreamble() {
	return QString::fromLatin1(glslVersionPrefix(currentContextIsGLES()))
	     + QStringLiteral(
		     "layout(location=0) in vec2 a_pos_;\n"
		     "layout(location=1) in vec2 a_uv_;\n"
		     "uniform mat4 u_mvp_;\n");
}

QString fragmentShimPreamble() {
	return QString::fromLatin1(glslVersionPrefix(currentContextIsGLES()))
	     + QStringLiteral("out vec4 frag_out_;\n");
}

// Pipeline: strip version → shared rewrites → stage rewrites → prepend preamble.
QString shimLegacyShader(QString src,
                         const std::vector<Rewrite>& stageRewrites,
                         const QString& preamble) {
	src = stripVersionDirective(std::move(src));
	src = applyRewrites(std::move(src), sharedRewrites());
	src = applyRewrites(std::move(src), stageRewrites);
	return preamble + src;
}

} // anonymous namespace

void MainWin::initializeGL() {
	qDebug() << __FUNCTION__;
	initializeOpenGLFunctions();
	conf.vid.shd_support = QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex) && QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment);
	curtex = 0;
	qDebug() << "vtx_shd";
	vtx_shd = new QOpenGLShader(QOpenGLShader::Vertex);
	qDebug() << "frg_shd";
	frg_shd = new QOpenGLShader(QOpenGLShader::Fragment);
	glGenTextures(4, texids);
	glEnable(GL_MULTISAMPLE);
	for (int i = 0; i < 4; i++) {
		glBindTexture(GL_TEXTURE_2D, texids[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	vao.create();
	vao.bind();
	vbo.create();
	vbo.bind();
	vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo.allocate(kQuadVertices, sizeof(kQuadVertices));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
	                      reinterpret_cast<void *>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
	                      reinterpret_cast<void *>(2 * sizeof(GLfloat)));
	vbo.release();
	vao.release();

	loadShader();
	if (!conf.vid.shd_support) qDebug() << "WARNING: Shaders not supported";
	qDebug() << "end:" << __FUNCTION__;
}

void MainWin::cleanupGL() {
#if !ISLEGACYGL
	if (!vao.isCreated() && !vbo.isCreated()) return;
	makeCurrent();
	if (vao.isCreated()) vao.destroy();
	if (vbo.isCreated()) vbo.destroy();
	prg.removeAllShaders();
	glDeleteTextures(4, texids);
	doneCurrent();
#endif
}

void MainWin::resizeGL(int w, int h) {
	const qreal r = widgetDpr(this);
	const int vw = int(w * r + 0.5);
	const int vh = int(h * r + 0.5);
	glViewport(0, 0, vw, vh);
	qDebug() << "resizeGL logical" << w << h << "dpr" << r << "viewport" << vw << vh;
}

void MainWin::paintGL() {
}

#endif

void MainWin::loadShader() {
#ifdef USEOPENGL
	if (!conf.vid.shd_support) return;

	QString vtx;
	QString frg;
	bool user_shader = false;

	if (!conf.vid.shader.empty()) {
		QString path = toQString(conf.path.find(ResourceKind::Shader, conf.vid.shader));
		QFile file(path);
		if (file.open(QFile::ReadOnly)) {
			int mode = 0;
			QString lin;
			while (!file.atEnd()) {
				lin = file.readLine().trimmed().append("\n");
				if (lin.startsWith("vertex:")) {
					mode = 1;
				} else if (lin.startsWith("fragment:")) {
					mode = 2;
				} else {
					switch (mode) {
						case 1: vtx.append(lin); break;
						case 2: frg.append(lin); break;
					}
				}
			}
			file.close();
			user_shader = true;
		} else {
			shitHappens("Can't open shader file");
		}
	}

	if (!user_shader) {
		vtx = composeShader(defaultVertexShaderBody());
		frg = composeShader(defaultFragmentShaderBody());
	} else if (sourceNeedsLegacyShim(vtx) || sourceNeedsLegacyShim(frg)) {
		vtx = shimLegacyShader(std::move(vtx), vertexRewrites(),   vertexShimPreamble());
		frg = shimLegacyShader(std::move(frg), fragmentRewrites(), fragmentShimPreamble());
	}

	prg.removeAllShaders();
	const bool vtx_ok = vtx_shd->compileSourceCode(vtx);
	if (!vtx_ok) qDebug() << "vertex shader:" << vtx_shd->log();
	const bool frg_ok = frg_shd->compileSourceCode(frg);
	if (!frg_ok) qDebug() << "fragment shader:" << frg_shd->log();

	if (vtx_ok && frg_ok) {
		prg.addShader(vtx_shd);
		prg.addShader(frg_shd);
		if (prg.link()) {
			if (user_shader) setMessage(" Shader compiled ");
			return;
		}
		qDebug() << "program link:" << prg.log();
	}

	if (user_shader) {
		setMessage(" Shader compile error ");
		conf.vid.shader.clear();
		loadShader();
	} else {
		qDebug() << "FATAL: default shader failed to compile/link";
	}
#endif
}

void MainWin::shdSelected(QAction* act) {
	conf.vid.shader = act->data().toString().toLocal8Bit().data();
	loadShader();
}
