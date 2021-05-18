//-------------------------------------------------------------------
#include "mainwindow.h"
#include "ui_mainwindow.h"
//-------------------------------------------------------------------
#include <QString>
#include <QThread>
//-------------------------------------------------------------------
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//-------------------------------------------------------------------
#define M_PI 3.1415926535897932384626433832795f
//-------------------------------------------------------------------
#define GET_CPU_TICK(cpu_tick) ({ __asm__ __volatile__ ("rdtsc" : "=A" (cpu_tick) : : ); })
//#define GET_CPU_TICK(cpu_tick) ({ __asm__ __volatile__ ("rdtsc" : "=A" (cpu_tick) : : "eax", "edx"); })
//-------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    platform_id(NULL),
    device_id(NULL),
    work_sizes(NULL),
    num_devices(0),
    num_platforms(0),
    status(0),
    dev_num(0)
{
    //-------------------------------------------------------------------------
    ui->setupUi(this);
    //-------------------------------------------------------------------------
    openCl_init();
    //-------------------------------------------------------------------------
//    dev_num = 0;
    openCl_calc_midle();
    //-------------------------------------------------------------------------
    cpu_calc_midle();
    //-------------------------------------------------------------------------
    qt_cpu_calc_midle.set_arr_size(arr_size);
    qt_cpu_calc_midle.set_midle_width(width);

    connect(&qt_cpu_calc_midle, SIGNAL(logMessage(QString)),
             this,              SLOT(onLogMessage(QString)) );

    qt_cpu_calc_midle.start();
    //-------------------------------------------------------------------------
    openCl_calc_float_midle();
    //-------------------------------------------------------------------------
    cpu_calc_float_midle();
    //-------------------------------------------------------------------------
    qt_cpu_calc_float_midle.set_arr_size(arr_size);
    qt_cpu_calc_float_midle.set_midle_width(width);

    connect(&qt_cpu_calc_float_midle, SIGNAL(logMessage(QString)),
             this,                    SLOT(onLogMessage(QString)) );

    qt_cpu_calc_float_midle.start();
    //-------------------------------------------------------------------------
    openCl_calc_xoz();
    //-------------------------------------------------------------------------
    cpu_calc_xoz();
    //-------------------------------------------------------------------------
    openCl_calc_yoz();
    //-------------------------------------------------------------------------
    cpu_calc_yoz();
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
MainWindow::~MainWindow()
{
    //-------------------------------------------------------------------------
//    qt_cpu_calc_midle.exit();

//    while (qt_cpu_calc_midle.isRunning());
    //-------------------------------------------------------------------------
//    qt_cpu_calc_float_midle.exit();

//    while (qt_cpu_calc_float_midle.isRunning());
    //-------------------------------------------------------------------------
    openCl_close();
    //-------------------------------------------------------------------------
    delete ui;
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::openCl_init(void)
{
    //-------------------------------------------------------------------------
    // OpenCL Platform ID
    //-------------------------------------------------------------------------
    status = clGetPlatformIDs(0, NULL, &num_platforms);
    platform_id = new cl_platform_id[num_platforms];
    status = clGetPlatformIDs(num_platforms, platform_id, NULL);

    ui->textBrowser->append(QString::fromUtf8("Обнаружено платформ: %1").arg(num_platforms));
    //-------------------------------------------------------------------------
    // OpenCL Platform Info
    //-------------------------------------------------------------------------
    //      CL_PLATFORM_PROFILE,
    //      CL_PLATFORM_VERSION,
    //      CL_PLATFORM_NAME,
    //      CL_PLATFORM_VENDOR,
    //      CL_PLATFORM_EXTENSIONS
    //-------------------------------------------------------------------------
    {
        uint i;
        for (i = 0; i < num_platforms; i++)
        {
            size_t  str_size;
            char*   str_val;
            //-------------------------------------------------------------------------
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_PROFILE, 0,  NULL, &str_size);
            str_val = new char[str_size];
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_PROFILE, str_size, str_val, NULL);
            ui->textBrowser->append(QString::fromUtf8("CL_PLATFORM_PROFILE: ") + QString(str_val));
            delete str_val;
            //-------------------------------------------------------------------------
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_VERSION, 0,  NULL, &str_size);
            str_val = new char[str_size];
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_VERSION, str_size, str_val, NULL);
            ui->textBrowser->append(QString::fromUtf8("CL_PLATFORM_VERSION: ") + QString(str_val));
            delete str_val;
            //-------------------------------------------------------------------------
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_NAME, 0,  NULL, &str_size);
            str_val = new char[str_size];
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_NAME, str_size, str_val, NULL);
            ui->textBrowser->append(QString::fromUtf8("CL_PLATFORM_NAME: ") + QString(str_val));
            delete str_val;
            //-------------------------------------------------------------------------
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_VENDOR, 0,  NULL, &str_size);
            str_val = new char[str_size];
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_VENDOR, str_size, str_val, NULL);
            ui->textBrowser->append(QString::fromUtf8("CL_PLATFORM_VENDOR: ") + QString(str_val));
            delete str_val;
            //-------------------------------------------------------------------------
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_EXTENSIONS, 0,  NULL, &str_size);
            str_val = new char[str_size];
            clGetPlatformInfo(platform_id[i], CL_PLATFORM_EXTENSIONS, str_size, str_val, NULL);
            ui->textBrowser->append(QString::fromUtf8("CL_PLATFORM_EXTENSIONS: ") + QString(str_val));
            delete str_val;
            //-------------------------------------------------------------------------
        }
    }
    //-------------------------------------------------------------------------
    // OpenCL Device ID
    //-------------------------------------------------------------------------
    //      CL_DEVICE_TYPE_GPU,
    //      CL_DEVICE_TYPE_CPU,
    //      . . .
    //      CL_DEVICE_TYPE_ALL
    //-------------------------------------------------------------------------
    {
//        device_id = new cl_device_id[num_platforms];
        uint i;
        for (i = 0; i < num_platforms; i++)
        {
            status = clGetDeviceIDs(platform_id[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
            device_id = new cl_device_id[num_devices];
            status = clGetDeviceIDs(platform_id[i], CL_DEVICE_TYPE_ALL, num_devices, device_id, NULL);
            ui->textBrowser->append(QString::fromUtf8("На платформе %1 обнаружено %2 OpenCL устройств").arg(i + 1).arg(num_devices));
        }
    }

    work_sizes = new size_t[num_devices];
    //-------------------------------------------------------------------------
    // OpenCL Device Info
    //-------------------------------------------------------------------------
    //      CL_DEVICE_TYPE
    //      CL_DEVICE_VENDOR_ID
    //      CL_DEVICE_MAX_COMPUTE_UNITS
    //      CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
    //      CL_DEVICE_MAX_WORK_GROUP_SIZE
    //      CL_DEVICE_MAX_WORK_ITEM_SIZES
    //      CL_DEVICE_MAX_CLOCK_FREQUENCY
    //-------------------------------------------------------------------------
    {
        uint i = 0;
        uint j;
//        for (i = 0; i < num_platforms; i++)
        {
            ui->textBrowser->append(QString::fromUtf8("Платформа %1").arg(i + 1));
            for (j = 0; j < num_devices; j++)
            {
                ui->textBrowser->append(QString::fromUtf8("OpenCL устройство #%1").arg(j + 1));
                //-------------------------------------------------------------------------
                size_t  str_size;
                char*   str_val;

                cl_device_type dev_type;
                status = clGetDeviceInfo(device_id[j], CL_DEVICE_TYPE, sizeof(dev_type), &dev_type, NULL);

                switch (dev_type)
                {
                    case CL_DEVICE_TYPE_CPU:
                        ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_TYPE = CL_DEVICE_TYPE_CPU"));
                        break;
                    case CL_DEVICE_TYPE_GPU:
                        ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_TYPE = CL_DEVICE_TYPE_GPU"));
                        break;
                    case CL_DEVICE_TYPE_ACCELERATOR:
                        ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_TYPE = CL_DEVICE_TYPE_ACCELERATOR"));
                        break;
                    case CL_DEVICE_TYPE_DEFAULT:
                        ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_TYPE = CL_DEVICE_TYPE_DEFAULT"));
                        break;
                    default:
                        break;
                }
                //-------------------------------------------------------------------------
                status = clGetDeviceInfo(device_id[j], CL_DEVICE_VENDOR, 0, NULL, &str_size);
                str_val = new char[str_size];
                status = clGetDeviceInfo(device_id[j], CL_DEVICE_VENDOR, str_size, str_val, NULL);
                ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_VENDOR - %1").arg(QString(str_val)));
                delete str_val;
                //-------------------------------------------------------------------------
                cl_uint mcu;
                status = clGetDeviceInfo(device_id[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(mcu), &mcu, NULL);
                ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_MAX_COMPUTE_UNITS - %1").arg(mcu));
                //-------------------------------------------------------------------------
                size_t wgs;
                status = clGetDeviceInfo(device_id[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(wgs), &wgs, NULL);
                ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_MAX_WORK_GROUP_SIZE - %1").arg(wgs));
                work_sizes[j] = wgs;
                //-------------------------------------------------------------------------
                cl_uint freq;
                status = clGetDeviceInfo(device_id[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(freq), &freq, NULL);
                ui->textBrowser->append(QString::fromUtf8("CL_DEVICE_MAX_CLOCK_FREQUENCY - %1 МГц").arg(freq));
                //-------------------------------------------------------------------------
            }

        }
    }
}
//-------------------------------------------------------------------
void MainWindow::openCl_close(void)
{

}
//-------------------------------------------------------------------------
// OpenCL device
//-------------------------------------------------------------------------
void MainWindow::openCl_calc_midle(void)
{
    int i;
    int *A;
    int *C;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    cl_device_id        work_dev_id;
    cl_context          context;
    cl_command_queue    command_queue;
    size_t              NDRange;            // здесь мы указываем размер вычислительной сетки
    size_t              work_size;          // размер рабочей группы (work-group)
    cl_mem              a_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              c_mem_obj;          // cl_mem это тип буфера памяти OpenCL
    cl_program          program;            // сюда будет записанна наша программа
    cl_kernel           kernel;             // сюда будет записан наш кернель
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу усреднения (%1 точек) на OpenCL устройство #%2 (целочисленные вычисления)").arg(width).arg(dev_num));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    A = (int *)malloc(sizeof(int) * arr_size); // выделяем место под массив А
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        A[i] = i;
    }
    //-------------------------------------------------------------------------
    C = (int *)malloc(sizeof(int) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        C[i] = 0;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    work_dev_id = device_id[dev_num];

    NDRange = arr_size;
    work_size = work_sizes[dev_num];//64;            // NDRange должен быть кратен размеру work-group
    //-------------------------------------------------------------------------
    context = clCreateContext(NULL, 1, &work_dev_id, NULL, NULL, &status);
//    command_queue = clCreateCommandQueueWithProperties(context, work_dev_id, NULL, &status);
    command_queue = clCreateCommandQueue(context, work_dev_id, 0, &status);
    //-------------------------------------------------------------------------
    a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_int), NULL, &status);
    c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, arr_size * sizeof(cl_int), NULL, &status);
    //-------------------------------------------------------------------------
    status = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, arr_size * sizeof(int), A, 0, NULL, NULL);  // записываем массив А
    //-------------------------------------------------------------------------
    program = clCreateProgramWithSource(context, 1, (const char**)&src_addition_str, NULL, &status);      // создаём программу из исходного кода
    status = clBuildProgram(program, 1, &work_dev_id, NULL, NULL, NULL);             // собираем программу (онлайн компиляция)
    //-------------------------------------------------------------------------
//    char log[0x10000];
//    clGetProgramBuildInfo( program, work_dev_id, CL_PROGRAM_BUILD_LOG, 0x10000, log, NULL);
//    ui->textBrowser->append(QString(log));
    //-------------------------------------------------------------------------
    kernel = clCreateKernel(program, "addition", &status);                         // создаём кернель
    //-------------------------------------------------------------------------
    status = clSetKernelArg(kernel, 0, sizeof(uint), (void *)&width);  // объект width
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&a_mem_obj);  // объект А
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);  // объект С
    //-------------------------------------------------------------------------
//    GET_CPU_TICK(tick1);
    status = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &NDRange, &work_size, 0, NULL, NULL); // исполняем кернель
//    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    status = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, arr_size * sizeof(int), C, 0, NULL, NULL); // записываем ответы
    //-------------------------------------------------------------------------
    status = clFlush(command_queue);                   // отчищаем очередь команд
    status = clFinish(command_queue);                  // завершаем выполнение всех команд в очереди
    status = clReleaseKernel(kernel);                  // удаляем кернель
    status = clReleaseProgram(program);                // удаляем программу OpenCL
    status = clReleaseMemObject(a_mem_obj);            // отчищаем OpenCL буфер А
    status = clReleaseMemObject(c_mem_obj);            // отчищаем OpenCL буфер С
    status = clReleaseCommandQueue(command_queue);     // удаляем очередь команд
    status = clReleaseContext(context);                // удаляем контекст OpenCL
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 <-> %2").arg(A[i]).arg(C[i]));
    }
    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(A);                                        // удаляем локальный буфер А
    free(C);                                        // удаляем локальный буфер С
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------------
// Вычисление при помощи CPU
//-------------------------------------------------------------------------
void MainWindow::cpu_calc_midle(void)
{
    int x;
    int i;
    int *A;
    int *C;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу усреднения (%1 точек) на CPU в обычном режиме (целочисленные вычисления)").arg(width));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    A = (int *)malloc(sizeof(int) * arr_size); // выделяем место под массив А
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        A[i] = i;
    }
    //-------------------------------------------------------------------------
    C = (int *)malloc(sizeof(int) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        C[i] = 0;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    for(x = 0; x < arr_size; x++)
    {
        int xlo  = x - (width >> 1);
        int xhi  = xlo + width;
        xlo = xlo < 0 ? 0 : xlo;
        xhi = xhi > (arr_size - 1) ? (arr_size - 1) : xhi;
        int sum = 0;
        int i;
        for (i = xlo; i < xhi; i++) sum += A[i];
        C[x] = sum / (xhi - xlo);
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 <-> %2").arg(A[i]).arg(C[i]));
    }
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(A);
    free(C);
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::openCl_calc_float_midle(void)
{
    int i;
    float *A;
    float *C;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    cl_device_id        work_dev_id;
    cl_context          context;
    cl_command_queue    command_queue;
    size_t              NDRange;            // здесь мы указываем размер вычислительной сетки
    size_t              work_size;          // размер рабочей группы (work-group)
    cl_mem              a_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              c_mem_obj;          // cl_mem это тип буфера памяти OpenCL
    cl_program          program;            // сюда будет записанна наша программа
    cl_kernel           kernel;             // сюда будет записан наш кернель
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу усреднения (%1 точек) на OpenCL устройство #%2 (вычисления с плавающей точкой)").arg(width).arg(dev_num));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    A = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив А
    for(i = 0; i < arr_size; i++)                  // наполняем массив данными
    {
        A[i] = (float)i;
    }
    //-------------------------------------------------------------------------
    C = (float *)malloc(sizeof(float) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)                  // наполняем массив данными
    {
        C[i] = 0.0f;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    work_dev_id = device_id[dev_num];

    NDRange = arr_size;
    work_size = work_sizes[dev_num];//64;            // NDRange должен быть кратен размеру work-group
    //-------------------------------------------------------------------------
    context = clCreateContext(NULL, 1, &work_dev_id, NULL, NULL, &status);
//    command_queue = clCreateCommandQueueWithProperties(context, work_dev_id, NULL, &status);
    command_queue = clCreateCommandQueue(context, work_dev_id, 0, &status);
    //-------------------------------------------------------------------------
    a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_float), NULL, &status);
    c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, arr_size * sizeof(cl_float), NULL, &status);
    //-------------------------------------------------------------------------
    status = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), A, 0, NULL, NULL);  // записываем массив А
    //-------------------------------------------------------------------------
    program = clCreateProgramWithSource(context, 1, (const char**)&src_float_addition_str, NULL, &status);      // создаём программу из исходного кода
    status = clBuildProgram(program, 1, &work_dev_id, NULL, NULL, NULL);             // собираем программу (онлайн компиляция)
    //-------------------------------------------------------------------------
//    char log[0x10000];
//    clGetProgramBuildInfo( program, work_dev_id, CL_PROGRAM_BUILD_LOG, 0x10000, log, NULL);
//    ui->textBrowser->append(QString(log));
    //-------------------------------------------------------------------------
    kernel = clCreateKernel(program, "float_addition", &status);                         // создаём кернель
    //-------------------------------------------------------------------------
    status = clSetKernelArg(kernel, 0, sizeof(uint), (void *)&width);  // объект width
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&a_mem_obj);  // объект А
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);  // объект С
    //-------------------------------------------------------------------------
//    GET_CPU_TICK(tick1);
    status = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &NDRange, &work_size, 0, NULL, NULL); // исполняем кернель
//    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    status = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), C, 0, NULL, NULL); // записываем ответы
    //-------------------------------------------------------------------------
    status = clFlush(command_queue);                   // отчищаем очередь команд
    status = clFinish(command_queue);                  // завершаем выполнение всех команд в очереди
    status = clReleaseKernel(kernel);                  // удаляем кернель
    status = clReleaseProgram(program);                // удаляем программу OpenCL
    status = clReleaseMemObject(a_mem_obj);            // отчищаем OpenCL буфер А
    status = clReleaseMemObject(c_mem_obj);            // отчищаем OpenCL буфер С
    status = clReleaseCommandQueue(command_queue);     // удаляем очередь команд
    status = clReleaseContext(context);                // удаляем контекст OpenCL
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 <-> %2").arg(A[i], 0, 'f', 3).arg(C[i], 0, 'f', 3));
    }
    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(A);                                        // удаляем локальный буфер А
    free(C);                                        // удаляем локальный буфер С
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::cpu_calc_float_midle(void)
{
    int x;
    int i;
    float *A;
    float *C;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу усреднения (%1 точек) на CPU в обычном режиме (вычисления с плавающей точкой)").arg(width));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    A = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив А
    for(i = 0; i < arr_size; i++)                  // наполняем массив данными
    {
        A[i] = (float)i;
    }
    //-------------------------------------------------------------------------
    C = (float *)malloc(sizeof(float) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)                  // наполняем массив данными
    {
        C[i] = 0.0f;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    for(x = 0; x < arr_size; x++)
    {
        int xlo  = x - (width >> 1);
        int xhi  = xlo + width;
        xlo = xlo < 0 ? 0 : xlo;
        xhi = xhi > (arr_size - 1) ? (arr_size - 1) : xhi;
        float sum = 0.0f;
        int i;
        for (i = xlo; i < xhi; i++) sum += A[i];
        C[x] = sum / (float)(xhi - xlo);
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 <-> %2").arg(A[i], 0, 'f', 3).arg(C[i], 0, 'f', 3));
    }
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(A);
    free(C);
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::openCl_calc_yoz(void)
{
    int i;
    float *X;
    float *Y;
    float *Z;
    float *YOZ;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    cl_device_id        work_dev_id;
    cl_context          context;
    cl_command_queue    command_queue;
    size_t              NDRange;            // здесь мы указываем размер вычислительной сетки
    size_t              work_size;          // размер рабочей группы (work-group)
    cl_mem              x_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              y_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              z_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              yoz_mem_obj;        // cl_mem это тип буфера памяти OpenCL
    cl_program          program;            // сюда будет записанна наша программа
    cl_kernel           kernel;             // сюда будет записан наш кернель
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу вычисления проекции угла YO'Z на OpenCL устройство #%1").arg(dev_num));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    X = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив X
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        X[i] = 0.1f * (float)( sin(0.000001f * M_PI * (float)i) );
    }
    //-------------------------------------------------------------------------
    Y = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Y
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Y[i] = 0.1f * (float)( cos(0.000001f * M_PI * (float)i));
    }
    //-------------------------------------------------------------------------
    Z = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Z
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Z[i] = (float)0.9f ;
    }
    //-------------------------------------------------------------------------
    YOZ = (float *)malloc(sizeof(float) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        YOZ[i] = (float)0.0f;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    work_dev_id = device_id[dev_num];

    NDRange = arr_size;
    work_size = work_sizes[dev_num];//64;            // NDRange должен быть кратен размеру work-group
    //-------------------------------------------------------------------------
    context = clCreateContext(NULL, 1, &work_dev_id, NULL, NULL, &status);
//    command_queue = clCreateCommandQueueWithProperties(context, work_dev_id, NULL, &status);
    command_queue = clCreateCommandQueue(context, work_dev_id, 0, &status);
    //-------------------------------------------------------------------------
    x_mem_obj   = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_float), NULL, &status);
    y_mem_obj   = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_float), NULL, &status);
    z_mem_obj   = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_float), NULL, &status);
    yoz_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, arr_size * sizeof(cl_float), NULL, &status);
    //-------------------------------------------------------------------------
    status = clEnqueueWriteBuffer(command_queue, x_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), X, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(command_queue, y_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), Y, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(command_queue, z_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), Z, 0, NULL, NULL);
    //-------------------------------------------------------------------------
    program = clCreateProgramWithSource(context, 1, (const char**)&src_gravity_yoz_str, NULL, &status);      // создаём программу из исходного кода
    status = clBuildProgram(program, 1, &work_dev_id, NULL, NULL, NULL);             // собираем программу (онлайн компиляция)
    //-------------------------------------------------------------------------
//    ui->textBrowser->append(QString(src_gravity_yoz_str));
//    char log[0x10000];
//    clGetProgramBuildInfo( program, work_dev_id, CL_PROGRAM_BUILD_LOG, 0x10000, log, NULL);
//    ui->textBrowser->append(QString(log));
    //-------------------------------------------------------------------------
    kernel = clCreateKernel(program, "gravity_yoz", &status);                         // создаём кернель
    //-------------------------------------------------------------------------
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&x_mem_obj);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&y_mem_obj);
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&z_mem_obj);
    status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&yoz_mem_obj);
    //-------------------------------------------------------------------------
    status = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &NDRange, &work_size, 0, NULL, NULL); // исполняем кернель
    //-------------------------------------------------------------------------
    status = clEnqueueReadBuffer(command_queue, yoz_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), YOZ, 0, NULL, NULL); // записываем ответы
    //-------------------------------------------------------------------------
    status = clFlush(command_queue);                   // отчищаем очередь команд
    status = clFinish(command_queue);                  // завершаем выполнение всех команд в очереди
    status = clReleaseKernel(kernel);                  // удаляем кернель
    status = clReleaseProgram(program);                // удаляем программу OpenCL
    status = clReleaseMemObject(x_mem_obj);
    status = clReleaseMemObject(y_mem_obj);
    status = clReleaseMemObject(z_mem_obj);
    status = clReleaseMemObject(yoz_mem_obj);
    status = clReleaseCommandQueue(command_queue);     // удаляем очередь команд
    status = clReleaseContext(context);                // удаляем контекст OpenCL
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(YOZ[i], 0, 'f', 3));
    }
    i = arr_size - 1;
    ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(YOZ[i], 0, 'f', 3));

    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(X);
    free(Y);
    free(Z);
    free(YOZ);
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::cpu_calc_yoz(void)
{
    int i;
    float *X;
    float *Y;
    float *Z;
    float *YOZ;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу вычисления проекции угла YO'Z на CPU в обычном режиме"));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    X = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив X
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        X[i] = 0.1f * (float)( sin(0.000001f * M_PI * (float)i) );
    }
    //-------------------------------------------------------------------------
    Y = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Y
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Y[i] = 0.1f * (float)( cos(0.000001f * M_PI * (float)i));
    }
    //-------------------------------------------------------------------------
    Z = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Z
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Z[i] = (float)0.9f ;
    }
    //-------------------------------------------------------------------------
    YOZ = (float *)malloc(sizeof(float) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        YOZ[i] = (float)0.0f;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    for(i = 0; i < arr_size; i++)
    {
        float sign;
        float x;
        float y;
        float z = Z[i];

        if (z > 0.0f)
        {
            x = X[i];
            if (x != 0.0f)
            {
                y = Y[i];
                if (y != 0.0f)
                {
                    if ((x*x + y*y + z*z) <= 2.0f)
                    {
                        if (x > 0.0f) sign = 1.0f;
                        else sign = -1.0f;
//                        dat->dept = grav1->dept;
                        YOZ[i] = 180.0f - sign * (180.0f * acos((y*z)/sqrt((x*x + y*y)*(x*x + z*z))) / M_PI);
                    }
                }
            }
        }
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(YOZ[i], 0, 'f', 3));
    }
    i = arr_size - 1;
    ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(YOZ[i], 0, 'f', 3));

    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(X);
    free(Y);
    free(Z);
    free(YOZ);
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::openCl_calc_xoz(void)
{
    int i;
    float *X;
    float *Y;
    float *Z;
    float *XOZ;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    cl_device_id        work_dev_id;
    cl_context          context;
    cl_command_queue    command_queue;
    size_t              NDRange;            // здесь мы указываем размер вычислительной сетки
    size_t              work_size;          // размер рабочей группы (work-group)
    cl_mem              x_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              y_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              z_mem_obj;          // опять, обратите внимание на тип данных
    cl_mem              xoz_mem_obj;        // cl_mem это тип буфера памяти OpenCL
    cl_program          program;            // сюда будет записанна наша программа
    cl_kernel           kernel;             // сюда будет записан наш кернель
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу вычисления проекции угла XO'Z на OpenCL устройство #%1").arg(dev_num));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    X = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив X
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        X[i] = 0.1f * (float)( sin(0.000001f * M_PI * (float)i) );
    }
    //-------------------------------------------------------------------------
    Y = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Y
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Y[i] = 0.1f * (float)( cos(0.000001f * M_PI * (float)i));
    }
    //-------------------------------------------------------------------------
    Z = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Z
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Z[i] = (float)0.9f ;
    }
    //-------------------------------------------------------------------------
    XOZ = (float *)malloc(sizeof(float) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        XOZ[i] = (float)0.0f;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    work_dev_id = device_id[dev_num];

    NDRange = arr_size;
    work_size = work_sizes[dev_num];//64;            // NDRange должен быть кратен размеру work-group
    //-------------------------------------------------------------------------
    context = clCreateContext(NULL, 1, &work_dev_id, NULL, NULL, &status);
//    command_queue = clCreateCommandQueueWithProperties(context, work_dev_id, NULL, &status);
    command_queue = clCreateCommandQueue(context, work_dev_id, 0, &status);
    //-------------------------------------------------------------------------
    x_mem_obj   = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_float), NULL, &status);
    y_mem_obj   = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_float), NULL, &status);
    z_mem_obj   = clCreateBuffer(context, CL_MEM_READ_ONLY,  arr_size * sizeof(cl_float), NULL, &status);
    xoz_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, arr_size * sizeof(cl_float), NULL, &status);
    //-------------------------------------------------------------------------
    status = clEnqueueWriteBuffer(command_queue, x_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), X, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(command_queue, y_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), Y, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(command_queue, z_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), Z, 0, NULL, NULL);
    //-------------------------------------------------------------------------
    program = clCreateProgramWithSource(context, 1, (const char**)&src_gravity_xoz_str, NULL, &status);      // создаём программу из исходного кода
    status = clBuildProgram(program, 1, &work_dev_id, NULL, NULL, NULL);             // собираем программу (онлайн компиляция)
    //-------------------------------------------------------------------------
//    ui->textBrowser->append(QString(src_gravity_xoz_str));
//    char log[0x10000];
//    clGetProgramBuildInfo( program, work_dev_id, CL_PROGRAM_BUILD_LOG, 0x10000, log, NULL);
//    ui->textBrowser->append(QString(log));
    //-------------------------------------------------------------------------
    kernel = clCreateKernel(program, "gravity_xoz", &status);                         // создаём кернель
    //-------------------------------------------------------------------------
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&x_mem_obj);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&y_mem_obj);
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&z_mem_obj);
    status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&xoz_mem_obj);
    //-------------------------------------------------------------------------
    status = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &NDRange, &work_size, 0, NULL, NULL); // исполняем кернель
    //-------------------------------------------------------------------------
    status = clEnqueueReadBuffer(command_queue, xoz_mem_obj, CL_TRUE, 0, arr_size * sizeof(float), XOZ, 0, NULL, NULL); // записываем ответы
    //-------------------------------------------------------------------------
    status = clFlush(command_queue);                   // отчищаем очередь команд
    status = clFinish(command_queue);                  // завершаем выполнение всех команд в очереди
    status = clReleaseKernel(kernel);                  // удаляем кернель
    status = clReleaseProgram(program);                // удаляем программу OpenCL
    status = clReleaseMemObject(x_mem_obj);
    status = clReleaseMemObject(y_mem_obj);
    status = clReleaseMemObject(z_mem_obj);
    status = clReleaseMemObject(xoz_mem_obj);
    status = clReleaseCommandQueue(command_queue);     // удаляем очередь команд
    status = clReleaseContext(context);                // удаляем контекст OpenCL
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(XOZ[i], 0, 'f', 3));
    }
    i = arr_size - 1;
    ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(XOZ[i], 0, 'f', 3));

    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(X);
    free(Y);
    free(Z);
    free(XOZ);
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::cpu_calc_xoz(void)
{
    int i;
    float *X;
    float *Y;
    float *Z;
    float *XOZ;
    quint64 tick1;
    quint64 tick2;
    //-------------------------------------------------------------------------
    ui->textBrowser->append(QString::fromUtf8("Выполняем программу вычисления проекции угла XO'Z на CPU в обычном режиме"));
    //-------------------------------------------------------------------------
    tick1 = 0;
    tick2 = 0;
    //-------------------------------------------------------------------------
    X = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив X
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        X[i] = 0.1f * (float)( sin(0.000001f * M_PI * (float)i) );
    }
    //-------------------------------------------------------------------------
    Y = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Y
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Y[i] = 0.1f * (float)( cos(0.000001f * M_PI * (float)i));
    }
    //-------------------------------------------------------------------------
    Z = (float *)malloc(sizeof(float) * arr_size); // выделяем место под массив Z
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        Z[i] = (float)0.9f ;
    }
    //-------------------------------------------------------------------------
    XOZ = (float *)malloc(sizeof(float) * arr_size); // выделяем память для массива с ответами
    for(i = 0; i < arr_size; i++)              // наполняем массив данными
    {
        XOZ[i] = (float)0.0f;
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick1);
    //-------------------------------------------------------------------------
    for(i = 0; i < arr_size; i++)
    {
        float sign;
        float x;
        float y;
        float z = Z[i];

        if (z > 0.0f)
        {
            x = X[i];
            if (x != 0.0f)
            {
                y = Y[i];
                if (y != 0.0f)
                {
                    if ((x*x + y*y + z*z) <= 2.0f)
                    {
                        if (y > 0.0f) sign = 1.0f;
                        else sign = -1.0f;
//                        dat->dept = grav1->dept;
                        XOZ[i] = 180.0f + sign * (180.0f * acos( (x*z)/sqrt((x*x + y*y)*(y*y + z*z)) ) / M_PI);
                    }
                }
            }
        }
    }
    //-------------------------------------------------------------------------
    GET_CPU_TICK(tick2);
    //-------------------------------------------------------------------------
    for(i = 0; i < 21; i++)
    {
        ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(XOZ[i], 0, 'f', 3));
    }
    i = arr_size - 1;
    ui->textBrowser->append(QString::fromUtf8("%1 | %2 | %3 <-> %4").arg(X[i], 0, 'f', 9).arg(Y[i], 0, 'f', 9).arg(Z[i], 0, 'f', 9).arg(XOZ[i], 0, 'f', 3));

    ui->textBrowser->append(QString::fromUtf8("Програма выполнена за %1 милионов тактов CPU").arg((tick2 - tick1)/1000000.0f, 0, 'f', 3));
    //-------------------------------------------------------------------------
    free(X);
    free(Y);
    free(Z);
    free(XOZ);
    //-------------------------------------------------------------------------
}
//-------------------------------------------------------------------
void MainWindow::onLogMessage(const QString message)
{
    ui->textBrowser->append(message);
}
//-------------------------------------------------------------------
