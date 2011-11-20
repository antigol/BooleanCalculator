#include "widget.h"
#include "ui_widget.h"
#include "booleancalcul.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(lineEditReturnPressed()));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(lineSelected(QListWidgetItem*)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::lineEditReturnPressed()
{
    QString command = ui->lineEdit->text();

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setText(command);
    item->setTextAlignment(Qt::AlignLeft);

    BooleanCalcul calcul(command);
    QString ret = calcul.karnaugh();
    if (calcul.error() != BooleanCalcul::None) {
        ret = "error";
    }

    item = new QListWidgetItem(ui->listWidget);
    item->setText(ret);
    item->setTextAlignment(Qt::AlignRight);
    ui->listWidget->setCurrentItem(item);
}

void Widget::lineSelected(QListWidgetItem *item)
{
    ui->lineEdit->setText(item->text());
}
