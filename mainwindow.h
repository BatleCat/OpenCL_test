//-------------------------------------------------------------------------
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//-------------------------------------------------------------------------
#include <QMainWindow>
#include <QString>
#include <QThread>
//-------------------------------------------------------------------------
#include "qt_cpu_calc_midle_class.h"
#include "qt_cpu_calc_float_midle_class.h"
//-------------------------------------------------------------------
#ifdef __APPLE__
# include <OpenCL/opencl.h>     // для компьютеров на MacOsX
#else
# include <CL/cl.h>             // для компьютеров на Win\Linux указывайте путь к файлу cl.h
#endif
#define MAX_SRC_SIZE (0x100000) // максимальный размер исходного кода кернеля
//-------------------------------------------------------------------
namespace Ui {
class MainWindow;
}
//-------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT
    //-------------------------------------------------------------------------
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //-------------------------------------------------------------------------
private:
    Ui::MainWindow *ui;
    //-------------------------------------------------------------------------
    // Усреднение по "width" точкам (целочисленные вычисления)
    const char  *src_addition_str =
            "__kernel void addition(const uint width, __global const int *A, __global int *C) \n"
            "{ \n"
            "    int x    = get_global_id(0); \n"
            "    int xdim = get_global_size(0); \n"
            "    int xlo  = x - (width >> 1); \n"
            "    int xhi  = xlo + width; \n"
            "    xlo = xlo < 0 ? 0 : xlo; \n"
            "    xhi = xhi > (xdim - 1) ? (xdim - 1) : xhi; \n"
            "    int sum = 0; \n"
            "    int i; \n"
            "    for (i = xlo; i < xhi; i++) sum += A[i]; \n"
            "    C[x] = sum / (xhi - xlo); \n"
            "} \n"
            " \n\0";
    //-------------------------------------------------------------------------
    // Усреднение по "width" точкам (вычисления с плавающей точкой)
    const char  *src_float_addition_str =
            "__kernel void float_addition(const uint width, __global const float *A, __global float *C) \n"
            "{ \n"
            "    __private int x    = get_global_id(0); \n"
            "    __private int xdim = get_global_size(0); \n"
            "    __private int xlo  = x - (width >> 1); \n"
            "    __private int xhi  = xlo + width; \n"
            "    xlo = xlo < 0 ? 0 : xlo; \n"
            "    xhi = xhi > (xdim - 1) ? (xdim - 1) : xhi; \n"
            "    __private float sum = 0.0f; \n"
            "    __private int i; \n"
            "    for (i = xlo; i < xhi; i++) sum += A[i]; \n"
            "    C[x] = sum / (float)(xhi - xlo); \n"
            "} \n"
            " \n\0";
    //-------------------------------------------------------------------------
    // проекции угла XO'Z
    const char  *src_gravity_xoz_str =
            "__kernel void gravity_xoz(__global const float *accelX, __global const float *accelY, __global const float *accelZ, __global float *xoz) \n"
            "{ \n"
            "   #define M_PI 3.1415926535897932384626433832795f \n"
            "   __private int i    = get_global_id(0); \n"
            "   __private float sign; \n"
            "   __private float x; \n"
            "   __private float y; \n"
            "   __private float z = accelZ[i]; \n"
            "   if (z > 0.0f) \n"
            "   { \n"
            "       x = accelX[i]; \n"
            "       if (x != 0.0f) \n"
            "       { \n"
            "           y = accelY[i]; \n"
            "           if (y != 0.0f) \n"
            "           { \n"
            "               if ((x*x + y*y + z*z) <= 2.0f) \n"
            "               { \n"
            "                   if (y > 0.0f) sign = 1.0f; \n"
            "                   else sign = -1.0f; \n"
//            dat->dept = grav1->dept;
            "                   xoz[i] = 180.0f + sign * (180.0f * acos( (x*z)/sqrt((x*x + y*y)*(y*y + z*z)) ) / M_PI); \n"
            "               } \n"
            "           } \n"
            "       } \n"
            "   } \n"
            "} \n"
            " \n\0";
    //-------------------------------------------------------------------------
    // проекции угла YO'Z
    const char  *src_gravity_yoz_str =
            "__kernel void gravity_yoz(__global const float *accelX, __global const float *accelY, __global const float *accelZ, __global float *yoz) \n"
            "{ \n"
            "   #define M_PI 3.1415926535897932384626433832795f \n"
            "   int i    = get_global_id(0); \n"
            "   float sign; \n"
            "   float x; \n"
            "   float y; \n"
            "   float z = accelZ[i]; \n"
            "   if (z > 0.0f) \n"
            "   { \n"
            "       x = accelX[i]; \n"
            "       if (x != 0.0f) \n"
            "       { \n"
            "           y = accelY[i]; \n"
            "           if (y != 0.0f) \n"
            "           { \n"
            "               if ((x*x + y*y + z*z) <= 2.0f) \n"
            "               { \n"
            "                   if (x > 0.0f) sign = 1.0f; \n"
            "                   else sign = -1.0f; \n"
//            dat->dept = grav1->dept;
            "                   yoz[i] = 180.0f - sign * ( 180.0f * acos( (y*z)/sqrt((x*x + y*y)*(x*x + z*z)) ) / M_PI ); \n"
            "               } \n"
            "           } \n"
            "       } \n"
            "   } \n"
            "} \n"
            " \n\0";
    //-------------------------------------------------------------------------
    const int  arr_size = 1024*1024;
    const uint width = 70;
    //-------------------------------------------------------------------------
    cl_platform_id* platform_id;    // обратите внимание на типы данных
    cl_device_id*   device_id;
    size_t*         work_sizes;     // размер рабочей группы (work-group)
    cl_uint         num_devices;
    cl_uint         num_platforms;
    cl_int          status;
    cl_uint         dev_num;

    qt_cpu_calc_midle_class qt_cpu_calc_midle;
//    QThread           	   *qt_cpu_calc_midle_thread;

    qt_cpu_calc_float_midle_class  qt_cpu_calc_float_midle;
//    QThread                       *qt_cpu_calc_float_midle_thread;
    //-------------------------------------------------------------------------
    void openCl_init(void);
    void openCl_close(void);
    void openCl_calc_midle(void);
    void cpu_calc_midle(void);
    void openCl_calc_float_midle(void);
    void cpu_calc_float_midle(void);
    void openCl_calc_yoz(void);
    void cpu_calc_yoz(void);
    void openCl_calc_xoz(void);
    void cpu_calc_xoz(void);
    //-------------------------------------------------------------------------
public slots:
    void onLogMessage(const QString message);
    //-------------------------------------------------------------------------

};
//-------------------------------------------------------------------------
#endif // MAINWINDOW_H
//-------------------------------------------------------------------------
