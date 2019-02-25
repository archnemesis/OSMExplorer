#ifndef TEXTLOGVIEWERFORM_H
#define TEXTLOGVIEWERFORM_H

#include <QWidget>

namespace Ui {
class TextLogViewerForm;
}

class TextLogViewerForm : public QWidget
{
    Q_OBJECT

public:
    explicit TextLogViewerForm(QWidget *parent = nullptr);
    ~TextLogViewerForm();

public slots:
    void addLine(const QString& line);

private:
    Ui::TextLogViewerForm *ui;
};

#endif // TEXTLOGVIEWERFORM_H
