#include <CL/cl.h>
#include <libclew/ocl_init.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

template <typename T> std::string to_string(T value) {
  std::ostringstream ss;
  ss << value;
  return ss.str();
}

void reportError(cl_int err, const std::string &filename, int line) {
  if (CL_SUCCESS == err)
    return;

  // Таблица с кодами ошибок:
  // libs/clew/CL/cl.h:103
  // P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже с
  // номером строки: cl.h:103) -> Enter
  std::string message = "OpenCL error code " + to_string(err) +
                        " encountered at " + filename + ":" + to_string(line);
  throw std::runtime_error(message);
}

#define OCL_SAFE_CALL(expr) reportError(expr, __FILE__, __LINE__)

template <typename T>
void PrintDeviceInfo(cl_device_id deviceId, cl_device_info paramId,
                     const char *paramTitle) {
  size_t paramSize = 0;
  OCL_SAFE_CALL(clGetDeviceInfo(deviceId, paramId, 0, nullptr, &paramSize));
  if (paramSize == sizeof(T)) {
    T paramValue;
    OCL_SAFE_CALL(clGetDeviceInfo(deviceId, paramId, sizeof(paramValue),
                                  &paramValue, nullptr));
    std::cout << "    " << paramTitle << ": " << paramValue << std::endl;
  } else
    std::cout << "    " << paramTitle << " has size " << paramSize
              << ", but given " << sizeof(T) << std::endl;
}

template<>
void PrintDeviceInfo<std::string>(cl_device_id deviceId, cl_device_info paramId,
                     const char *paramTitle) {
  size_t paramSize = 0;
  OCL_SAFE_CALL(clGetDeviceInfo(deviceId, paramId, 0, nullptr, &paramSize));
  std::vector<unsigned char> paramValue(paramSize, 0);
  OCL_SAFE_CALL(clGetDeviceInfo(deviceId, paramId, paramSize, &paramValue[0],
                                &paramSize));
  std::cout << "    " << paramTitle << ": " << paramValue.data() << std::endl;
}

int main() {

  try {
    // Пытаемся слинковаться с символами OpenCL API в runtime (через библиотеку
    // libs/clew)
    if (!ocl_init())
      throw std::runtime_error("Can't init OpenCL driver!");

    // Откройте
    // https://www.khronos.org/registry/OpenCL/sdk/1.2/docs/man/xhtml/
    // Нажмите слева: "OpenCL Runtime" -> "Query Platform Info" ->
    // "clGetPlatformIDs" Прочитайте документацию clGetPlatformIDs и убедитесь,
    // что этот способ узнать, сколько есть платформ, соответствует
    // документации:
    cl_uint platformsCount = 0;
    OCL_SAFE_CALL(clGetPlatformIDs(0, nullptr, &platformsCount));
    std::cout << "Number of OpenCL platforms: " << platformsCount << std::endl;

    // Тот же метод используется для того, чтобы получить идентификаторы всех
    // платформ - сверьтесь с документацией, что это сделано верно:
    std::vector<cl_platform_id> platforms(platformsCount);
    OCL_SAFE_CALL(clGetPlatformIDs(platformsCount, platforms.data(), nullptr));

    for (int platformIndex = 0; platformIndex < platformsCount;
         ++platformIndex) {
      std::cout << "Platform #" << (platformIndex + 1) << "/" << platformsCount
                << std::endl;
      cl_platform_id platform = platforms[platformIndex];

      auto PrintPlatformInfo = [&platform](cl_platform_info paramId,
                                           const char *paramTitle) {
        // Откройте документацию по "OpenCL Runtime" -> "Query Platform Info" ->
        // "clGetPlatformInfo" Не забывайте проверять коды ошибок с помощью
        // макроса OCL_SAFE_CALL

        size_t paramSize = 0;
        OCL_SAFE_CALL(
            clGetPlatformInfo(platform, paramId, 0, nullptr, &paramSize));

        // TODO 1.1
        // Попробуйте вместо CL_PLATFORM_NAME передать какое-нибудь случайное
        // число - например 239 Т.к. это некорректный идентификатор параметра
        // платформы - то метод вернет код ошибки Макрос OCL_SAFE_CALL заметит
        // это, и кинет ошибку с кодом Откройте таблицу с кодами ошибок:
        // libs/clew/CL/cl.h:103
        // P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже
        // с номером строки: cl.h:103) -> Enter Найдите там нужный код ошибки и
        // ее название Затем откройте документацию по clGetPlatformInfo и в
        // секции Errors найдите ошибку, с которой столкнулись в документации
        // подробно объясняется, какой ситуации соответствует данная ошибка, и
        // это позволит, проверив код, понять, чем же вызвана данная ошибка
        // (некорректным аргументом param_name) Обратите внимание, что в этом же
        // libs/clew/CL/cl.h файле указаны всевоможные defines, такие как
        // CL_DEVICE_TYPE_GPU и т.п.

        // TODO 1.2
        // Аналогично тому, как был запрошен список идентификаторов всех
        // платформ
        // - так и с названием платформы, теперь, когда известна длина названия
        // - его можно запросить:
        std::vector<unsigned char> paramValue(paramSize, 0);
        OCL_SAFE_CALL(clGetPlatformInfo(platform, paramId, paramSize,
                                        &paramValue[0], &paramSize));
        std::cout << "    " << paramTitle << ": " << paramValue.data()
                  << std::endl;
      };

      PrintPlatformInfo(CL_PLATFORM_NAME, "CL_PLATFORM_NAME");
      // TODO 1.3
      PrintPlatformInfo(CL_PLATFORM_VENDOR, "CL_PLATFORM_VENDOR");
      PrintPlatformInfo(CL_PLATFORM_VERSION, "CL_PLATFORM_VERSION");
      PrintPlatformInfo(CL_PLATFORM_PROFILE, "CL_PLATFORM_PROFILE");
      PrintPlatformInfo(CL_PLATFORM_EXTENSIONS, "CL_PLATFORM_EXTENSIONS");

      // TODO 2.1
      // Запросите число доступных устройств данной платформы (аналогично тому,
      // как это было сделано для запроса числа доступных платформ - см. секцию
      // "OpenCL Runtime" -> "Query Devices")
      cl_uint devicesCount = 0;

      OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr,
                                   &devicesCount));
      std::cout << "Number of OpenCL devices: " << devicesCount << std::endl;

      std::vector<cl_device_id> devices(devicesCount);
      OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, devicesCount,
                                   &devices[0], &devicesCount));

      for (int deviceIndex = 0; deviceIndex < devicesCount; ++deviceIndex) {
        // TODO 2.2
        // Запросите и напечатайте в консоль:
        // - Название устройства
        // - Тип устройства (видеокарта/процессор/что-то странное)
        // - Размер памяти устройства в мегабайтах
        // - Еще пару или более свойств устройства, которые вам покажутся
        // наиболее интересными

        std::cout << "Device #" << (deviceIndex + 1) << "/" << devicesCount
                  << std::endl;

        PrintDeviceInfo<std::string>(devices[deviceIndex], CL_DEVICE_NAME, "CL_DEVICE_NAME");
        PrintDeviceInfo<std::string>(devices[deviceIndex], CL_DEVICE_VENDOR, "CL_DEVICE_VENDOR");
        PrintDeviceInfo<cl_device_type>(devices[deviceIndex], CL_DEVICE_TYPE, "CL_DEVICE_TYPE");

      }
    }
  } catch (std::exception &e) {
    std::cout << "Exception " << e.what() << std::endl;
  }
  return 0;
}