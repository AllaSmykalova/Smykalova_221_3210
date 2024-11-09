#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

struct Move {
    int i;
    int j;
    QString dateTime;
    QString hash;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_buttonClicked();
    void on_pushButton_load_clicked();
    void on_pushButton_clear_clicked();

private:
    Ui::MainWindow *ui;
    int clickCount;
    QVector<QPushButton*> gameButtons;
    QVector<Move> moves;
    QString previousHash;
    void saveMovesToFile();
};

#endif // MAINWINDOW_H
