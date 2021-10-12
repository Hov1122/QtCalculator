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
{
    ui->setupUi(this);
    qApp->installEventFilter(this); // to track keyPresses

    ///hide advanced math functions
    ui->op_exp->setVisible(false);
    ui->fun_sin->setVisible(false);
    ui->fun_cos->setVisible(false);
    ui->fun_ln->setVisible(false);
    ui->fun_sqrt->setVisible(false);
    ui->op_exp->setEnabled(false);
    ui->fun_sin->setEnabled(false);
    ui->fun_cos->setEnabled(false);
    ui->fun_ln->setEnabled(false);
    ui->fun_sqrt->setEnabled(false);

   // QRegularExpression rx("[^a-zA-Z&^$#@!\[_\\] ={};:\"']+"); //exclude this charactes
    QRegularExpression rx("[0-9/\\^*%()+-.]+"); // allow only those characters
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->input->setValidator(validator);
    ui->input->setMaxLength(35);

    /// set up button's connections

    for (int i = 0; i <  ui->numbersOpLayout->columnCount() * ui->numbersOpLayout->rowCount(); i++)
    { // -1 cuz first row has only columnCount() - 1 widgets

        QWidget* widget = ui->numbersOpLayout->itemAt(i)->widget();
        QPushButton* button = qobject_cast<QPushButton*>(widget);

        if (button && (button == ui->op_add || button == ui->op_sub || button == ui->op_percentage ||
                       button == ui->op_div || button == ui->op_mul || button == ui->op_exp))
            connect(button, SIGNAL(clicked()), this, SLOT(op_mathOpClicked()));

        else if (button && (button == ui->fun_sin || button == ui->fun_ln || button == ui->fun_cos
                            || button == ui->fun_sqrt))
                 connect(button, SIGNAL(clicked()), this, SLOT(mathFunClicked()));

        else if ( button && (button != ui->op_brackets && button != ui->op_equal && button != ui->op_clear
                             && button != ui->op_point))
        {
            connect( button, SIGNAL(clicked()), this, SLOT(numClicked()) );
        }
    }

    connect(ui->op_clear, SIGNAL(clicked()), this, SLOT(op_clearClicked()));
    connect(ui->op_del, SIGNAL(clicked()), this, SLOT(op_delClicked()));
    connect(ui->op_equal, SIGNAL(clicked()), this, SLOT(op_equalClicked()));
    connect(ui->op_brackets, SIGNAL(clicked()), this, SLOT(op_bracketsClicked()));
    connect(ui->op_point, SIGNAL(clicked()), this, SLOT(op_pointClicked()));
    connect(ui->advanced_fun, SIGNAL(clicked()), this, SLOT(advanced_funClicked()));
    connect(ui->input, SIGNAL(textChanged(const QString&)), this, SLOT(on_input_textChanged(const QString&)));
}

Calculator::~Calculator()
{
    delete ui;
}

void Calculator::numClicked()
{
    QPushButton* callingButton = qobject_cast<QPushButton*>( sender() );
    int cp = ui->input->cursorPosition();
    if (callingButton) {
        if (cp > 0 && ui->input->text()[cp - 1] == ')') ui->input->setText(ui->input->text().insert(cp++, '*'));
        ui->input->setText(ui->input->text().insert(cp++, callingButton->text()));
        if (cp < ui->input->text().size() && ui->input->text()[cp] == '(') ui->input->setText(ui->input->text().insert(cp++, '*'));
        ui->input->setCursorPosition(cp);
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
    ui->input->setFocus();

    if (!ui->input->text().isEmpty()) {
        if (ui->input->hasSelectedText()) // if has selected text delete it
        {
            int st = ui->input->selectionStart();
            int end = ui->input->selectionEnd();
            if (st > end) std::swap(st, end);
            ui->input->setText(ui->input->text().remove(st, end - st));
            ui->input->setCursorPosition(st);
            return;
        }
        int cp = ui->input->cursorPosition();
        if (cp > 0) {
            ui->input->setText(ui->input->text().remove(cp - 1, 1));
            ui->input->setCursorPosition(cp - 1);
        }
    } 
}

/// allow point(.) to be inputted if and only if previous symbol is digit and that number doesnt contain other point symbol
void Calculator::op_pointClicked()
{
    if (!ui->input->text().isEmpty()) {

        int cp = ui->input->cursorPosition();
        if (!ui->input->text()[cp - 1].isDigit()) return;

        bool pointAllowed1 = true;
        bool pointAllowed2 = true;
        QString ip = ui->input->text();
        for (int tmp = cp - 1; tmp >= 0; tmp--)
        {
            if (ip[tmp].isDigit()) continue;
            else if (ip[tmp] == '.')
            {
                pointAllowed1 = false;
                break;
            }
            else
            {
                pointAllowed1 = true;
                break;
            }
        }
        if (pointAllowed1)
        {
            for (int tmp = cp; tmp < ui->input->text().size(); tmp++)
            {
                if (ip[tmp].isDigit()) continue;
                else if (ip[tmp] == '.')
                {
                    pointAllowed2 = false;
                    break;
                }
                else
                {
                    pointAllowed2 = true;
                    break;
                }
            }
        }

        bool pointAllowed = pointAllowed1 && pointAllowed2;

        if (pointAllowed) {
            ui->input->setText(ui->input->text().insert(cp, ui->op_point->text()));
            ui->input->setCursorPosition(cp + 1);
        }
    }
    ui->input->setFocus();
}

/// allow math operator to be inputted if and only if previous symbol isnt math operator or point except }
void Calculator::op_mathOpClicked()
{
    ui->input->setFocus();

    QPushButton* callingButton = qobject_cast<QPushButton*>( sender() );

    int cp = ui->input->cursorPosition();

    if (cp == 0 && callingButton == ui->op_sub) {
        ui->input->setText(ui->input->text().insert(cp, callingButton->text()));
        ui->input->setCursorPosition(cp + 1);
        return;
    }
    if (ui->input->text().isEmpty()) return;

    QChar notAllowed[] = {'/', '%', '*', '+', '-', '^', '.', '('};
    bool allowed = true;

    for (int i = 0; i < 8; i++)
        if (i == 7) break;
        else if (cp < ui->input->text().size() && ui->input->text()[cp] == notAllowed[i]) {
            allowed = false;
            break;
        }

    for (int i = 0; i < 8; i++)
        if (i == 7 && callingButton == ui->op_sub) break;
        else if (cp > 0 && ui->input->text()[cp - 1] == notAllowed[i]) {
            allowed = false;
            break;
        }

    if (allowed) {
        //ui->input->setText(ui->input->text() + callingButton->text());
        ui->input->setText(ui->input->text().insert(cp, callingButton->text()));
        ui->input->setCursorPosition(cp + 1);
    }
}

void Calculator::mathFunClicked(){
    ui->input->setFocus();

    QPushButton* callingButton = qobject_cast<QPushButton*>( sender() );

    int cp = ui->input->cursorPosition();

    if (cp > 0 && ui->input->text()[cp - 1].isLetter()) {
        return;
    }

    if (cp > 0 &&
            (ui->input->text()[cp - 1].isDigit() || ui->input->text()[cp - 1] == ')')) {
        ui->input->setText(ui->input->text().insert(cp++, "*"));
    }

    ui->input->setText(ui->input->text().insert(cp, callingButton->text()));
    cp += callingButton->text().size();
    ui->input->setText(ui->input->text().insert(cp, "()"));
    cp += 2;

    if (cp < ui->input->text().size() && (ui->input->text()[cp].isDigit() || ui->input->text()[cp] == '(')) {
        ui->input->setText(ui->input->text().insert(cp, "*"));
    }

    ui->input->setCursorPosition(cp - 1);

}

/// cacluate the input and output result in answer label
void Calculator::op_equalClicked()
{
    ui->input->setFocus();

    if (ui->input->text().isEmpty()){
        return;
    }
    QChar no = ui->input->text()[ui->input->text().size() - 1];
    QString res;
    if (no == '/' || no == '*' || no == '+' || no == '-' || no == '%' || no == '.') {
        res = "Invalid input";
        ui->answerLabel->setStyleSheet("QLabel { color : red; }");
    }
    else {
        res = countExpression(ui->input->text());
        if (res == "Invalid input") ui->answerLabel->setStyleSheet("QLabel { color : red; }");
        else ui->answerLabel->setStyleSheet("QLabel { color : black; }");
    }
    ui->answerLabel->setText(res);  
}

/// inputs () to lineEdit and adds * before it if it follows to digit
void Calculator::op_bracketsClicked()
{
    int cp = ui->input->cursorPosition();
    if (cp > 0 && (ui->input->text()[cp - 1].isDigit() || ui->input->text()[cp - 1] == ')')) {
        ui->input->setText(ui->input->text().insert(cp, "*"));
        cp++;
    }

    ui->input->setText(ui->input->text().insert(cp++, ui->op_brackets->text()));
    if (cp < ui->input->text().size() - 1 && (ui->input->text()[cp + 1].isDigit() || ui->input->text()[cp - 1] == '(')) {
        ui->input->setText(ui->input->text().insert(++cp, "*"));
    }
    ui->input->setCursorPosition(cp - 1);
    ui->input->setFocus();
}

/// returns precendence of arihmetic operators
int Calculator::precedence(char op){
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/' || op == '%')
        return 2;
    return 0;
}

// Function to perform arithmetic operations. return Invalid input if expression is not valid
double Calculator::applyOp(double a, double b, char op){
    switch(op){
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
        case '%': return b * a / 100;
        case '^': return pow(a, b);
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

        // stack to store function names
        std::stack <std::string> functions;

        // stack to store operators.
        std::stack <char> ops;

        bool negative = false; // to determine whether number is negative or not
        bool bracketsEmpty = false;

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
                bracketsEmpty = true;
            }

            else if (isalpha(exp[i])) {
                if (exp[i] == 'l') {
                    if (i + 4 < exp.size() && exp.substr(i, 3) == "ln(") // min is ln(m)
                    {
                        functions.push("ln");
                        i += 1; // i will point to ( after next iteration
                    }
                    else return "Invalid input";
                }

                else if (exp[i] == 's') {
                    if (i + 5 < exp.size() && exp.substr(i, 4) == "sin(") // min is sin(m)
                    {
                        functions.push("sin");
                        i += 2;
                    }

                    else if (i + 6 < exp.size() && exp.substr(i, 5) == "sqrt(") // ,in is sqrt(
                    {
                        functions.push("sqrt");
                        i += 3;
                    }
                    else return "Invalid input";
                }

                else if (exp[i] == 'c') {
                    if (i + 5 < exp.size() && exp.substr(i, 4) == "cos(") // min is cos(m)
                    {
                        functions.push("cos");
                        i += 2;
                    }
                    else return "Invalid input";
                }

                else
                {
                    return "Invalid input";
                }


            }

            else if((i == 0 || exp[i - 1] == '(') && exp[i] == '-')
            {
                negative = true;
            }

            // Current token is a number, push
            // it to stack for numbers.
            else if(isdigit(exp[i]))
            {
                bracketsEmpty = false;
                double val = 0;
                double rm = 0;
                bool q = false;
                bool noStZero = (exp[i] == '0'); // 01234 not valid
                if (noStZero) i++;
                // There may be more than one
                // digits in number.
                int j = 1;
                while(i < exp.length() &&
                            (isdigit(exp[i]) || exp[i] == '.'))
                {
                    if (q)
                    {
                        if (exp[i] == '.') return "Invalid input";
                        rm += (exp[i] - '0') / pow(10, j);
                        j++;
                    }

                    else
                    {
                        if (exp[i] != '.' && !noStZero)
                            val = (val*10) + (exp[i]-'0');
                    }

                    if (exp[i] == '.') {
                        q = true;
                        noStZero = false;
                    }

                    else if (noStZero) return "Invalid input";
                    i++;
                }

                if (q) val += rm;
                if (negative) {
                    val *= -1;
                    negative = false;
                }

                values.push(val);

                // right now the i points to
                // the character next to the digit,
                // since the for loop also increases
                // the i, we would skip one
                // token position; we need to
                // decrease the value of i by 1 to
                // correct the offset.
                  i--;
            }

            // Closing brace encountered, solve
            // entire brace.
            else if(exp[i] == ')')
            {
                if (bracketsEmpty) return "Invalid input";
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
                    if (val2 == 0 && op == '/') return "Invalid input";
                    values.push(applyOp(val1, val2, op));
                }

                if (!functions.empty()) {
                    std::string tmp = functions.top();
                    functions.pop();
                    if (values.empty()) return "Invalid input";
                    double val = values.top();
                    values.pop();
                    if (tmp == "ln")
                    {
                        if (val <= 0) return "Invalid input";
                        values.push(log(val));
                    }
                    else if (tmp == "sin") values.push(sin(val * M_PI / 180));
                    else if (tmp == "cos") {
                        if ((int)val % 90 == 0 && (int)val % 180 != 0) values.push(0);
                        else values.push(cos(val * M_PI / 180));
                    }
                    else if (tmp == "sqrt") values.push(sqrt(val));
                }

                // pop opening brace.
                if(!ops.empty())
                   ops.pop();
                else return ("Invalid input");
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
                    if (val2 == 0 && op == '/') return "Invalid input";
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
            if (val2 == 0 && op == '/') return "Invalid input";
            values.push(applyOp(val1, val2, op));
        }

        // Top of 'values' contains result, return it.
        if (values.size() != 1) return "Invalid input";
        return QString::number(values.top(), 'g', 12);
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
                //emit(ui->op_sub->clicked());
                ui->op_sub->animateClick();
                return true;
            }

            else if (keyEvent->key() == Qt::Key_Slash)
            {
               // emit(ui->op_div->clicked());
                ui->op_div->animateClick();
                return true;
            }

            else if (keyEvent->modifiers() == Qt::ShiftModifier && keyEvent->key() == Qt::Key_Backspace)
            {
                /// assign shift + backspace to clear button
                //emit(ui->op_clear->clicked());
                ui->op_clear->animateClick();
                return true;
            }

            else if (keyEvent->modifiers() == Qt::AltModifier && keyEvent->key() == Qt::Key_A)
            {
                /// assign shift + backspace to clear button
                //emit(ui->op_clear->clicked());
                ui->advanced_fun->animateClick();
                return true;
            }

            else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || key == "=") {
                //emit(ui->op_equal->clicked());
                ui->op_equal->animateClick();
                return true;
            }

            else if (key == "*") {
               // emit(ui->op_mul->clicked());
                ui->op_mul->animateClick();
                return true;
            }

            else if (key == "%") {
               // emit(ui->op_percentage->clicked());
                ui->op_percentage->animateClick();
                return true;
            }

            else if (key == "^") {
               // emit(ui->op_percentage->clicked());
                ui->op_exp->animateClick();
                return true;
            }

            else if (key == "+") {
               // emit(ui->op_add->clicked());
                ui->op_add->animateClick();
                return true;
            }

            else if (key == ".") {
              //  emit(ui->op_point->clicked());
                ui->op_point->animateClick();
                return true;
            }

            else if (key == "(")
            {
                int cp = ui->input->cursorPosition();
                if (cp > 0 && (ui->input->text()[cp - 1].isDigit() || ui->input->text()[cp - 1] == ')'))
                {
                    ui->input->setText(ui->input->text().insert(cp, "*"));
                    cp++;
                }
                ui->input->setText(ui->input->text().insert(cp++, "("));
                ui->input->setCursorPosition(cp);

                return true;
            }

            else if (key == ")")
            {
                /// add * after ) if next symbol is digit
                int cp = ui->input->cursorPosition();
                if (cp < ui->input->text().size() && ui->input->text()[cp].isDigit())
                {
                    ui->input->setText(ui->input->text().insert(cp++, ")*"));
                    ui->input->setCursorPosition(++cp);
                    return true;
                }
            }

            else if (keyEvent->key() == Qt::Key_Backspace) {
                ui->op_del->setAutoRepeat(true);
                ui->op_del->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->op_del->animateClick();
                return true;
            }

            else if (key == '0') {
              //  emit(ui->num_1->clicked());
                ui->num_0->setAutoRepeat(true);
                ui->num_0->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_0->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '1') {
              //  emit(ui->num_1->clicked());
                ui->num_1->setAutoRepeat(true);
                ui->num_1->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_1->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '2') {
              //  emit(ui->num_1->clicked());
                ui->num_2->setAutoRepeat(true);
                ui->num_2->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_2->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '3') {
              //  emit(ui->num_1->clicked());
                ui->num_3->setAutoRepeat(true);
                ui->num_3->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_3->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '4') {
              //  emit(ui->num_1->clicked());
                ui->num_4->setAutoRepeat(true);
                ui->num_4->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_4->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '5') {
              //  emit(ui->num_1->clicked());
                ui->num_5->setAutoRepeat(true);
                ui->num_5->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_5->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '6') {
              //  emit(ui->num_1->clicked());
                ui->num_6->setAutoRepeat(true);
                ui->num_6->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_6->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '7') {
              //  emit(ui->num_1->clicked());
                ui->num_7->setAutoRepeat(true);
                ui->num_7->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_7->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '8') {
              //  emit(ui->num_1->clicked());
                ui->num_8->setAutoRepeat(true);
                ui->num_8->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_8->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
            }

            else if (key == '9') {
              //  emit(ui->num_1->clicked());
                ui->num_9->setAutoRepeat(true);
                ui->num_9->setAutoRepeatInterval(40); // to continue calling click function until key is released
                ui->num_9->animateClick();
                //while (ui->num_1->isDown()) ui->num_1->animateClick();
                return true;
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


void Calculator::advanced_funClicked()
{
    ui->input->setFocus();

    if (!ui->fun_ln->isVisible()) {
        QRegularExpression rx("[0-9/\\^*%()+-.lnsqrtico]+"); // allow only those characters
        QValidator *validator = new QRegularExpressionValidator(rx, this);
        ui->input->setValidator(validator);
        ui->fun_ln->setVisible(true);
        ui->fun_ln->setEnabled(true);
        ui->fun_sin->setVisible(true);
        ui->fun_sin->setEnabled(true);
        ui->fun_cos->setVisible(true);
        ui->fun_cos->setEnabled(true);
        ui->fun_sqrt->setVisible(true);
        ui->fun_sqrt->setEnabled(true);
        ui->op_exp->setVisible(true);
        ui->op_exp->setEnabled(true);
    }
    else {
        QRegularExpression rx("[0-9/\\*%()+-.]+"); // allow only those characters
        QValidator *validator = new QRegularExpressionValidator(rx, this);
        ui->input->setValidator(validator);
        ui->fun_ln->setVisible(false);
        ui->fun_ln->setEnabled(false);
        ui->fun_sin->setVisible(false);
        ui->fun_sin->setEnabled(false);
        ui->fun_cos->setVisible(false);
        ui->fun_cos->setEnabled(false);
        ui->fun_sqrt->setVisible(false);
        ui->fun_sqrt->setEnabled(false);
        ui->op_exp->setVisible(false);
        ui->op_exp->setEnabled(false);
    }
}

