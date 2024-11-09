#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    clickCount(0)
{
    ui->setupUi(this);
    setWindowTitle("Блокчейн-игра Смыкалова");

    // Инициализация игрового поля
    for (int i = 1; i <= 16; ++i) {
        QPushButton *button = findChild<QPushButton*>(QString("pushButton_%1").arg(i));
        if (button) {
            gameButtons.append(button);
            connect(button, &QPushButton::clicked, this, &MainWindow::on_buttonClicked);
        }
    }

    // Подключаем обработчики для кнопок "Загрузить" и "Сброс"
    connect(ui->pushButton_load, &QPushButton::clicked, this, &MainWindow::on_pushButton_load_clicked);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &MainWindow::on_pushButton_clear_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button || !button->isEnabled()) return;

    // Увеличиваем счетчик нажатий
    clickCount++;

    // Меняем цвет в зависимости от четности нажатия
    if (clickCount % 2 == 0) {
        button->setStyleSheet("background-color: green");
    } else {
        button->setStyleSheet("background-color: red");
    }

    // Отключаем кнопку после нажатия
    button->setEnabled(false);

    // Получаем индексы клетки
    int index = gameButtons.indexOf(button);
    int i = index / 4;
    int j = index % 4;

    // Получаем текущее время
    QString dateTime = QDateTime::currentDateTime().toString("yyyy.MM.dd_HH:mm:ss");

    // Формируем строку для хеширования
    QString dataToHash = QString::number(i) + QString::number(j) + dateTime + previousHash;

    // Рассчитываем SHA-256 хеш
    QByteArray hashData = QCryptographicHash::hash(dataToHash.toUtf8(), QCryptographicHash::Sha256);
    QString hashString = hashData.toHex();

    // Сохраняем хеш для следующего хода
    previousHash = hashString;

    // Записываем ход
    Move move;
    move.i = i;
    move.j = j;
    move.dateTime = dateTime;
    move.hash = hashString;
    moves.append(move);

    // Сохраняем состояние игры в файл
    saveMovesToFile();
}

void MainWindow::saveMovesToFile() {
    QJsonArray moveArray;
    foreach (const Move &move, moves) {
        QJsonObject moveObject;
        moveObject["i"] = QString::number(move.i);
        moveObject["j"] = QString::number(move.j);
        moveObject["dateTime"] = move.dateTime;
        moveObject["hash"] = move.hash;
        moveArray.append(moveObject);
    }

    QJsonDocument doc(moveArray);
    QByteArray jsonData = doc.toJson();

    QFile file("../../game_state.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonData);
        file.close();
    }
}

void MainWindow::on_pushButton_load_clicked()
{
    QString currentPath = QDir::currentPath();
    qDebug() << "Current path: " << currentPath;

    QFile file("../../game_state.json");
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to parse JSON data.");
        return;
    }

    QJsonArray moveArray = doc.array();
    moves.clear();
    previousHash.clear();
    int moveCount = 0;

    foreach (const QJsonValue &value, moveArray) {
        moveCount++;
        QJsonObject moveObject = value.toObject();
        Move move;
        move.i = moveObject["i"].toString().toInt();
        move.j = moveObject["j"].toString().toInt();
        move.dateTime = moveObject["dateTime"].toString();
        move.hash = moveObject["hash"].toString();

        // Проверка хеш-суммы
        QString dataToHash = QString::number(move.i) + QString::number(move.j) + move.dateTime + previousHash;
        QByteArray hashData = QCryptographicHash::hash(dataToHash.toUtf8(), QCryptographicHash::Sha256);
        QString calculatedHash = hashData.toHex();

        if (calculatedHash != move.hash) {
            QMessageBox::warning(this, "Hash Error", QString("Hash mismatch at move %1").arg(moveCount));
            return;
        }

        // Обновление previousHash
        previousHash = move.hash;

        // Восстановление состояния игры
        int index = move.i * 4 + move.j;
        QPushButton *button = gameButtons.at(index);
        if (moveCount % 2 == 0) {
            button->setStyleSheet("background-color: green");
        } else {
            button->setStyleSheet("background-color: red");
        }
        button->setEnabled(false);
    }
}

void MainWindow::on_pushButton_clear_clicked()
{
    foreach (QPushButton *button, gameButtons) {
        button->setEnabled(true);
        button->setStyleSheet("");
    }
    moves.clear();
    previousHash.clear();
}
