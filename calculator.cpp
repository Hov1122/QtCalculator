#include "calculator.h"
#include "ui_calculator.h"
#include <iostream>

Calculator::Calculator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);

    for (int i = 0; i <  ui->numbersOpLayout->columnCount() * ui->numbersOpLayout->rowCount(); i++)
    {

        QWidget* widget = ui->numbersOpLayout->itemAt( i )->widget();
        QPushButton* button = qobject_cast<QPushButton*>( widget );

        if (button && (button == ui->op_add || button == ui->op_sub || button == ui->op_percentage ||
                       button == ui->op_div || button == ui->op_mul))
            connect(button, SIGNAL(clicked()), this, SLOT(op_mathOpClicked()));

        else if (button && button == ui->op_sign)
            connect(button, SIGNAL(clicked()), this, SLOT(op_signClicked()));

        else if ( button && (button != ui->op_brackets && button != ui->op_equal && button != ui->op_clear))
        {
            connect( button, SIGNAL(clicked()), this, SLOT(numOpNonSpecialClicked()) );
        }
    }

    connect(ui->op_clear, SIGNAL(clicked()), this, SLOT(op_clearClicked()));
    connect(ui->op_del, SIGNAL(clicked()), this, SLOT(op_delClicked()));
    connect(ui->op_equal, SIGNAL(clicked()), this, SLOT(op_equalClicked()));
}

Calculator::~Calculator()
{
    delete ui;
}


void Calculator::numOpNonSpecialClicked()
{
    QPushButton* callingButton = qobject_cast<QPushButton*>( sender() );
    if (callingButton)
        ui->input->setText(ui->input->text() + callingButton->text());
}


void Calculator::op_clearClicked()
{
    ui->input->clear();
}

void Calculator::op_delClicked()
{
    if (!ui->input->text().isEmpty())
        ui->input->setText(ui->input->text().remove(ui->input->text().size() - 1, 1));
}

void Calculator::op_mathOpClicked()
{
    if (ui->input->text().isEmpty()) return;

    QPushButton* callingButton = qobject_cast<QPushButton*>( sender() );

    QChar notAllowed[] = {'/', '%', '*', '+', '-', '.'};
    bool allowed = true;
    for (int i = 0; i < 6; i++)
        if (ui->input->text()[ui->input->text().size() - 1] == notAllowed[i])
            allowed = false;

    if (allowed)
        ui->input->setText(ui->input->text() + callingButton->text());
}

void Calculator::op_signClicked()
{

}

void Calculator::op_equalClicked()
{
    double res = countExpression(ui->input->text());
    ui->answerLabel->setText(QString::number(res));
}

double Calculator::countExpression(const QString& exp)
{

}
