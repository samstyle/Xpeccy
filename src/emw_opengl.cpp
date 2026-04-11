#include "emulwin.h"

#if defined(USEOPENGL) && !BLOCKGL

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

} // anonymous namespace

void MainWin::initializeGL() {
	qDebug() << __FUNCTION__;
#if !ISLEGACYGL
	initializeOpenGLFunctions();
	conf.vid.shd_support = QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex) && QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment);
	curtex = 0;
	qDebug() << "vtx_shd";
	vtx_shd = new QOpenGLShader(QOpenGLShader::Vertex);
	qDebug() << "frg_shd";
	frg_shd = new QOpenGLShader(QOpenGLShader::Fragment);
#else
//	QGLFormat frmt;
//	frmt.setDoubleBuffer(false);
//	cont = new QGLContext(frmt);
//	setContext(cont);
//	setAutoBufferSwap(true);
//	makeCurrent();
//	curtex = 0;
//	conf.vid.shd_support = QGLShader::hasOpenGLShaders(QGLShader::Vertex) && QGLShader::hasOpenGLShaders(QGLShader::Fragment);
//	qDebug() << "vtx_shd";
//	vtx_shd = new QGLShader(QGLShader::Vertex, cont);
//	qDebug() << "frg_shd";
//	frg_shd = new QGLShader(QGLShader::Fragment, cont);
#endif
	glGenTextures(4, texids);
	glEnable(GL_MULTISAMPLE);
	for (int i = 0; i < 4; i++) {
		glBindTexture(GL_TEXTURE_2D, texids[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

#if !ISLEGACYGL
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
#endif

	loadShader();
	if (!conf.vid.shd_support) qDebug() << "WARNING: Shaders not supported";
	qDebug() << "end:" << __FUNCTION__;
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
#if defined(USEOPENGL) && !BLOCKGL
	if (!conf.vid.shd_support) return;

	QString vtx;
	QString frg;
	bool user_shader = false;

	if (!conf.vid.shader.empty()) {
		QString path(std::string(conf.path.shdDir + SLASH + conf.vid.shader).c_str());
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
