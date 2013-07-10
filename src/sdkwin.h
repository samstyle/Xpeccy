#ifndef SDKWIN_H
#define SDKWIN_H

#include <QMainWindow>
#include <QModelIndex>
#include <QProcess>
#include <QSyntaxHighlighter>

class MSyn : public QSyntaxHighlighter {
	Q_OBJECT
	public:
	MSyn(QObject* p):QSyntaxHighlighter(p) {}
	protected:
		void highlightBlock(const QString &text);
};

class SDKWindow : public QMainWindow {
	Q_OBJECT
	public:
		SDKWindow(QWidget* p = NULL);
	private:
		void savePrjFile();
		void buildTree();
		QProcess prc;
	private slots:
		void newProject();
		void openProject();
		void saveProject();

		void buildProject();
		void buildFinish();

//		void addExFiles();

		void prjFileChanged(QModelIndex);
		void textChanged();
	protected:
		void closeEvent(QCloseEvent*);
};

void devInit();
void devShow();

extern QString prjDir;
extern QString compPath;

#endif // SDKWIN_H
