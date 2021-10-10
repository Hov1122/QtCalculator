#include <QKeyEvent>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <stack>
#include <cmath>
#include "calculator.h"
#include "ui_calculator.h"

Calculator::Calculator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Calculator)
    , pointAllowed(true)
{
    ui->setupUi(this);
    qApp->installEventFilter(this); // to track keyPresses

   // QRegularExpression rx("[^a-zA-Z&^$#@!\[_\\] ={};:\"']+"); //exclude this charactes
    QRegularExpression rx("[0-9/*%()+-.]+"); // allow only those characters
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->input->setValidator(validator);
    ui->input->setMaxLength(35);

    /// set up button's connections

    for (int i = 0; i <  ui->numbersOpLayout->columnCount() * ui->numbersOpLayout->rowCount() - 1; i++)
    { // -1 cuz first row has only columnCount() - 1 widgets

        QWidget* widget = ui->numbersOpLayout->itemAt( i )->widget();
        QPushButton* button = qobject_cast<QPushButton*>( widget );

        if (button && (button == ui->op_add || button == ui->op_sub || button == ui->op_percentage ||
                       button == ui->op_div || button == ui->op_mul))
            connect(button, SIGNAL(clicked()), this, SLOT(op_mathOpClicked()));

        else if ( button && (button != ui->op_brackets && button != ui->op_equal && button != ui->op_clear
                             && button != ui->op_point))
        {
            connect( button, SIGNAL(clicked()), this, SLOT(numOpNonSpecialClicked()) );
        }
    }

    connect(ui->op_clear, SIGNAL(clicked()), this, SLOT(op_clearClicked()));
    connect(ui->op_del, SIGNAL(clicked()), this, SLOT(op_delClicked()));
    connect(ui->op_equal, SIGNAL(clicked()), this, SLOT(op_equalClicked()));
    connect(ui->op_brackets, SIGNAL(clicked()), this, SLOT(op_bracketsClicked()));
    connect(ui->op_point, SIGNAL(clicked()), this, SLOT(op_pointClicked()));
    connect(ui->input, SIGNAL(textChanged(const QString&)), this, SLOT(on_input_textChanged()));

}

Calculator::~Calculator()
{
    delete ui;
}


void Calculator::numOpNonSpecialClicked()
{
    QPushButton* callingButton = qobject_cast<QPushButton*>( sender() );
    int cp = ui->input->cursorPosition();
    if (callingButton) {
        if (ui->input->text()[cp - 1] == ')') ui->input->setText(ui->input->text().insert(cp++, '*'));
        ui->input->setText(ui->input->text().insert(cp, callingButton->text()));
        ui->input->setCursorPosition(cp + 1);
    }
    ui->input->setFocus();
}

void Calculator::op_clearClicked()
{
    ui->input->clear();
    ui->input->setFocus(); // set focus to input
}

void Calculator::op_delClicked()
{
    if (!ui->input->text().isEmpty()) {
        int cp = ui->input->cursorPosition();
        if (cp > 0) {
            ui->input->setText(ui->input->text().remove(cp - 1, 1));
            ui->input->setCursorPosition(cp - 1);
        }
    }
    ui->input->setFocus();
}

/// allow point(.) to be inputted if and only if previous symbol is digit and that number doesnt contain other point symbol
void Calculator::op_pointClicked()
{
    if (!ui->input->text().isEmpty()) {
        int cp = ui->input->cursorPosition();
        if (ui->input->text()[cp - 1].isDigit() && pointAllowed) {
            ui->input->setText(ui->input->text().insert(cp, ui->op_point->text()));
            ui->input->setCursorPosition(cp + 1);
            pointAllowed = false;
        }
    }
    ui->input->setFocus();
}

/// allow math operator to be inputted if and only if previous symbol isnt math operator or point except }
void Calculator::op_mathOpClicked()
{
    if (ui->input->text().isEmpty()) return;
    QPushButton* callingButton = qobject_cast<QPushButton*>( sender() );

    QChar notAllowed[] = {'/', '%', '*', '+', '-', '.', '('};
    bool allowed = true;
    int cp = ui->input->cursorPosition();

    for (int i = 0; i < 7; i++)
        if (i == 6 && callingButton == ui->op_sub) break;
        else if (ui->input->text()[cp - 1] == notAllowed[i])
            allowed = false;

    if (allowed) {
        //ui->input->setText(ui->input->text() + callingButton->text());
        ui->input->setText(ui->input->text().insert(cp, callingButton->text()));
        ui->input->setCursorPosition(cp + 1);
    }
    ui->input->setFocus();
    pointAllowed = true;
}

/// cacluate the input and output result in answer label
void Calculator::op_equalClicked()
{
    if (ui->input->text().isEmpty()){
        ui->input->setFocus();
        return;
    }
    QChar no = ui->input->text()[ui->input->text().size() - 1];
    QString res;
    if (no == '/' || no == '*' || no == '+' || no == '-' || no == '%') {
        res = "Invalid input";
        ui->answerLabel->setStyleSheet("QLabel { color : red; }");
    }
    else {
        res = countExpression(ui->input->text());
        if (res == "Invalid input") ui->answerLabel->setStyleSheet("QLabel { color : red; }");
        else ui->answerLabel->setStyleSheet("QLabel { color : black; }");
    }
    ui->answerLabel->setText(res);
    ui->input->setFocus();
}

/// inputs () to lineEdit and adds * before it if it follows to digit
void Calculator::op_bracketsClicked()
{
    int cp = ui->input->cursorPosition();
    if (cp > 0 && ui->input->text()[cp - 1].isDigit()) {
        ui->input->setText(ui->input->text().insert(cp, "*"));
        cp++;
    }

    ui->input->setText(ui->input->text().insert(cp, ui->op_brackets->text()));
    ui->input->setCursorPosition(cp + 1);
    ui->input->setFocus();
}

/// returns precendence of arihmetic operators
int precedence(char op){
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/' || op == '%')
        return 2;
    return 0;
}

// Function to perform arithmetic operations. return Invalid input if expression is not valid
double applyOp(double a, double b, char op){
    switch(op){
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
        case '%': return b * a / 100;
    }
    return 0;
}

/// main function to calculate the input
QString Calculator::countExpression(const QString& ex)
{

        int i;
        std::string exp = ex.toStdString();
        // stack to store integer values.
        std::stack <double> values;

        // stack to store operators.
        std::stack <char> ops;



        for(i = 0; i < exp.length(); i++)
        {

            // Current token is a whitespace,
            // skip it.
            if(exp[i] == ' ')
                continue;
            // Current token is an opening
            // brace, push it to 'ops'
            else if(exp[i] == '(')
            {
                ops.push(exp[i]);
            }

            // Current token is a number, push
            // it to stack for numbers.
            else if(isdigit(exp[i]))
            {
                double val = 0;
                double rm = 0;
                bool q = false;
                // There may be more than one
                // digits in number.
                int j = 1;
                while(i < exp.length() &&
                            (isdigit(exp[i]) || exp[i] == '.'))
                {
                    if (q)
                    {
                        rm += (exp[i] - '0') / pow(10, j);
                        j++;
                    }
                    else
                    {
                        if (exp[i] != '.')
                            val = (val*10) + (exp[i]-'0');
                    }
                    if (exp[i] == '.') q = true;
                    i++;
                }

                if (q) val += rm;
                values.push(val);

                // right now the i points to
                // the character next to the digit,
                // since the for loop also increases
                // the i, we would skip one
                //  token position; we need to
                // decrease the value of i by 1 to
                // correct the offset.
                  i--;
            }

            // Closing brace encountered, solve
            // entire brace.
            else if(exp[i] == ')')
            {

                while(!ops.empty() && ops.top() != '(')
                {
                    if (values.empty()) return "Invalid input";
                    double val2 = values.top();
                    values.pop();

                    if (values.empty()) return "Invalid input";
                    double val1 = values.top();
                    values.pop();

                    char op = ops.top();
                    ops.pop();

                    if (op == '(' || op == ')') return "Invalid input";
                    values.push(applyOp(val1, val2, op));
                }

                // pop opening brace.
                if(!ops.empty())
                   ops.pop();
                else return ("Ivalid input");
            }

            // Current token is an operator.
            else
            {
                // While top of 'ops' has same or greater
                // precedence to current token, which
                // is an operator. Apply operator on top
                // of 'ops' to top two elements in values stack.
                while(!ops.empty() && precedence(ops.top())
                                    >= precedence(exp[i])){
                    if (values.empty()) return "Invalid input";
                    double val2 = values.top();
                    values.pop();

                    if (values.empty()) return "Invalid input";
                    double val1 = values.top();
                    values.pop();

                    char op = ops.top();
                    ops.pop();

                    if (op == '(' || op == ')') return "Invalid input";
                    values.push(applyOp(val1, val2, op));
                }

                // Push current token to 'ops'.
                ops.push(exp[i]);
            }
        }

        // Entire expression has been parsed at this
        // point, apply remaining ops to remaining
        // values.
        while(!ops.empty()){
            if (values.empty()) return "Invalid input";
            double val2 = values.top();
            values.pop();

            if (values.empty()) return "Invalid input";
            double val1 = values.top();
            values.pop();

            char op = ops.top();
            ops.pop();

            if (op == '(' || op == ')') return "Invalid input";
            values.push(applyOp(val1, val2, op));
        }

        // Top of 'values' contains result, return it.
        if (values.size() != 1) return "Invalid input";
        return QString::number(values.top());
}

/// overwrides evenFilter function to proceed some key presses as needed
bool Calculator::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->input)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            QString key;

            key = QKeySequence(keyEvent->key()).toString();
            //qDebug().nospace() << "abc" << qPrintable(key) << "def";

            if (keyEvent->key() == 45) // for -
            {
                emit(ui->op_sub->clicked());
                ui->op_sub->animateClick();
                return true;
            }

            else if (keyEvent->key() == Qt::Key_Slash)
            {
                emit(ui->op_div->clicked());
                ui->op_div->animateClick();
                return true;
            }

            else if (keyEvent->modifiers() == Qt::ShiftModifier && keyEvent->key() == Qt::Key_Backspace)
            {
                /// assign shift + backspace to clear button
                emit(ui->op_clear->clicked());
                ui->op_clear->animateClick();
                return true;
            }

            else if (keyEvent->key() == Qt::Key_Return) {
                emit(ui->op_equal->clicked());
                ui->op_equal->animateClick();
                return true;
            }

            else if (key == "*") {
                emit(ui->op_mul->clicked());
                ui->op_mul->animateClick();
                return true;
            }

            else if (key == "%") {
                emit(ui->op_percentage->clicked());
                ui->op_percentage->animateClick();
                return true;
            }

            else if (key == "+") {
                emit(ui->op_add->clicked());
                ui->op_add->animateClick();
                return true;
            }

            else if (key == ".") {
                emit(ui->op_point->clicked());
                ui->op_point->animateClick();
                return true;
            }

            else if (key == "=") {
                emit(ui->op_equal->clicked());
                ui->op_equal->animateClick();
                return true;
            }

            else if (key == "(")
            {
                int cp = ui->input->cursorPosition();
                if (cp > 0 && ui->input->text()[cp - 1].isDigit())
                {
                    ui->input->setText(ui->input->text().insert(cp, "*"));
                    cp++;
                }
                ui->input->setText(ui->input->text().insert(cp, "("));

                return true;
            }

            else {
                /// add * after ) if next symbol is digit
                std::string tmp = key.toStdString();
                if (isdigit(tmp[0])) {
                    int cp = ui->input->cursorPosition();
                    if (cp > 0 && ui->input->text()[cp - 1] == ')')
                    {
                        ui->input->setText(ui->input->text().insert(cp++, "*"));
                        ui->input->setText(ui->input->text().insert(cp++, QString::number(tmp[0] - '0')));
                        cp++;
                        return true;
                    }

                }
            }

        }
    }
    // pass the event on to the parent class

    return QMainWindow::eventFilter(obj, event);
}

/// clear answer label if input is changed
void Calculator::on_input_textChanged(const QString&)
{
    ui->answerLabel->clear();
}

