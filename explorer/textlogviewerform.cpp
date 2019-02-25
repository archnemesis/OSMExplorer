#include "textlogviewerform.h"
#include "ui_textlogviewerform.h"

TextLogViewerForm::TextLogViewerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextLogViewerForm)
{
    ui->setupUi(this);
}

TextLogViewerForm::~TextLogViewerForm()
{
    delete ui;
}

void TextLogViewerForm::addLine(const QString &line)
{
    ui->pteTextView->appendPlainText(QString("%1\n").arg(line));
}
