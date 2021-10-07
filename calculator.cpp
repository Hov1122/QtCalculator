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

        if ( button && (button != ui->op_brackets && button != ui->op_equal && button != ui->op_clear
                        && button != ui->op_sign))
        {
            connect( button, SIGNAL(clicked()), this, SLOT(numOpNonSpecialClicked()) );
        }
    }
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

