#include "emulwin.h"

#ifdef USEOPENGL

void MainWin::initializeGL() {
	qDebug() << __FUNCTION__;
#if !ISLEGACY
	QSurfaceFormat frmt;
	frmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	frmt.setSwapInterval(0);
	cont = new QOpenGLContext(windowHandle());
	cont->create();
	cont->setFormat(frmt);
	Q_ASSERT(context() != nullptr);
	context()->setShareContext(cont);
	initializeOpenGLFunctions();
	shd_support = QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex) && QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment);
	curtex = 0;
	qDebug() << "vtx_shd";
	vtx_shd = new QOpenGLShader(QOpenGLShader::Vertex, cont);		// ERROR: create QOpenGLFunctions with non-current context
	qDebug() << "frg_shd";
	frg_shd = new QOpenGLShader(QOpenGLShader::Fragment, cont);
#endif
	glGenTextures(4, texids);	// create texture
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_MULTISAMPLE);
	for (int i = 0; i < 4; i++) {
		// select texture
		glBindTexture(GL_TEXTURE_2D, texids[i]);
		// set filters for this texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	qDebug() << "end:" << __FUNCTION__;
}

void MainWin::resizeGL(int w, int h) {
	qDebug() << __FUNCTION__ << w << h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 1.0, 0.0, 1, 0);
	glMatrixMode(GL_MODELVIEW);
	qDebug() << "end:" << __FUNCTION__;
}

#endif

void MainWin::loadShader() {
#ifdef USEOPENGL
	QString path(std::string(conf.path.shdDir + SLASH + conf.vid.shader).c_str());
	QFile file(path);
	int mode = 0;
	QString vtx;
	QString frg;
	QString lin;
	prg.removeAllShaders();
	if (!conf.vid.shader.empty() && shd_support) {			// no shader selected
		if (file.open(QFile::ReadOnly)) {
			while(!file.atEnd()) {
				lin = file.readLine().trimmed().append("\n");
				if (lin.startsWith("vertex:")) {
					mode = 1;
				} else if (lin.startsWith("fragment:")) {
					mode = 2;
				} else {
					switch(mode) {
						case 1: vtx.append(lin); break;
						case 2: frg.append(lin); break;
					}
				}
			}
			file.close();
			if (!vtx.isEmpty()) {
				if (!vtx_shd->compileSourceCode(vtx)) {
					qDebug() << vtx_shd->log();
				}
			}
			if (!frg.isEmpty()) {
				if (!frg_shd->compileSourceCode(frg)) {
					qDebug() << frg_shd->log();
				}
			}
			if (vtx_shd->isCompiled() && frg_shd->isCompiled()) {
				prg.addShader(vtx_shd);
				prg.addShader(frg_shd);
				prg.link();
				setMessage(" Shader compiled ");
			} else {
				setMessage(" Shader compile error ");
				conf.vid.shader.clear();
				loadShader();
			}
		} else {
			shitHappens("Can't open shader file");
		}
	}
#endif
}

void MainWin::shdSelected(QAction* act) {
	conf.vid.shader = act->data().toString().toLocal8Bit().data();
	loadShader();
}
