#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Calculator; }
QT_END_NAMESPACE

class Calculator : public QMainWindow
{
    Q_OBJECT

public:
    Calculator(QWidget *parent = nullptr);
    ~Calculator();

private slots:
    void numOpNonSpecialClicked();
    void op_clearClicked();
    void op_delClicked();
    void op_mathOpClicked();
    void op_signClicked();

private:
    Ui::Calculator *ui;
};
#endif // CALCULATOR_H
