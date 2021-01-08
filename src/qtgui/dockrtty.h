#ifndef DOCKRTTY_H
#define DOCKRTTY_H
#include <QDockWidget>
#include <QSettings>

namespace Ui {
    class DockRTTY;
}


class DockRTTY : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockRTTY(QWidget *parent = 0);
    ~DockRTTY();

public slots:
    void update_text(QString text);
    void show_Enabled();
    void show_Disabled();
    void set_Enabled();
    void set_Disabled();
    void set_baud_rate(float);
    void set_mark_freq(float);
    void set_space_freq(float);

private:
    void ClearText();
    float d_baud_rate;
    float d_mark_freq;
    float d_space_freq;
    int d_mode;
    int d_parity;

signals:
    void rtty_start_decoder();
    void rtty_stop_decoder();
    void rtty_baud_rate_Changed(float);
    void rtty_mark_freq_Changed(float);
    void rtty_space_freq_Changed(float);
    void rtty_mode_Changed(int);
    void rtty_parity_Changed(int);

private slots:
    void on_rttyCheckBox_toggled(bool);

    void on_baud_rate_editingFinished();
    void on_mark_freq_editingFinished();
    void on_space_freq_editingFinished();
    void on_mode_currentIndexChanged(int); 
    void on_parity_currentIndexChanged(int); 

private:
    Ui::DockRTTY *ui;        /*! The Qt designer UI file. */
};

#endif // DOCKRTTY_H
