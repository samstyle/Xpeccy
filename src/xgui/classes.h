#pragma once

#include <QAbstractTableModel>
#include <QDockWidget>
#include <QLabel>
#include <QProxyStyle>

class xTableModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xTableModel(QObject* = NULL);
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
		void setRows(int);
		void setCols(int);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
	public slots:
		void update();
		void updateRow(int);
		void updateColumn(int);
		void updateCell(int, int);
	protected:
		int row_count;
		int col_count;
	private:
		int mrd(int);
		void mwr(int, int);
};

class xDockWidget : public QDockWidget {
	Q_OBJECT
	public:
		xDockWidget(QString = "", QString = "", QWidget* = nullptr);
		QList<int> hwList;		// Hardware groups for showing this widget in deBUGa
	private:
		QIcon icon;
		QString title;
	protected:
		QLabel* titleWidget;
	public slots:
		virtual void draw() {}
		void moved();
};
