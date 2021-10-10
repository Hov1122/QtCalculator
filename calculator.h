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
    void numClicked();
    void op_clearClicked();
    void op_delClicked();
    void op_mathOpClicked();
    void op_equalClicked();
    void op_bracketsClicked();
    void op_pointClicked();
    void on_input_textChanged(const QString&);

private:
    Ui::Calculator *ui;

    bool pointAllowed; // false if number already contains point(.)
    bool eventFilter(QObject *obj, QEvent *event);
    QString countExpression(const QString&);
};
#endif // CALCULATOR_H
