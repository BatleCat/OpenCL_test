//-------------------------------------------------------------------------
#ifndef QT_CPU_CALC_MIDLE_CLASS_H
#define QT_CPU_CALC_MIDLE_CLASS_H
//-------------------------------------------------------------------------
#include <QObject>
#include <QThread>
#include <QString>
//-------------------------------------------------------------------------
class qt_cpu_calc_midle_class : public QThread
{
    Q_OBJECT
public:
    explicit qt_cpu_calc_midle_class();

    void set_arr_size(int new_size) { arr_size = new_size; }
    int  get_arr_size(void)         { return arr_size; }

    void set_midle_width(uint new_width) { width = new_width; }
    uint get_midle_width(void)           { return width; }

    void run();

private:
    int  arr_size;
    uint width;

signals:
    void logMessage(const QString message);
};
//-------------------------------------------------------------------------
#endif // QT_CPU_CALC_MIDLE_CLASS_H
//-------------------------------------------------------------------------
