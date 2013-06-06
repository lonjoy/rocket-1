#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_mainwindow.h"

class TrackArea;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

private:
	Ui::MainWindowClass ui;

	TrackArea *trackArea;
};

#endif // MAINWINDOW_H
