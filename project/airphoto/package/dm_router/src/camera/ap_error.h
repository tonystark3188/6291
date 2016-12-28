#ifndef __AP_ERROR_H
#define __AP_ERROR_H


#define GP_OK                            0
/**
 * \brief Generic Error
 */
#define GP_ERROR                        -1
/**
 * \brief Bad parameters passed
 */
#define GP_ERROR_BAD_PARAMETERS		-2
/**
 * \brief Out of memory
 */
#define GP_ERROR_NO_MEMORY		-3
/**
 * \brief Error in the camera driver
 */
#define GP_ERROR_LIBRARY		-4
/**
 * \brief Unknown libgphoto2 port passed
 */
#define GP_ERROR_UNKNOWN_PORT		-5
/**
 * \brief Functionality not supported
 */
#define GP_ERROR_NOT_SUPPORTED		-6
/**
 * \brief Generic I/O error
 */
#define GP_ERROR_IO			-7
/**
 * \brief Buffer overflow of internal structure
 */
#define GP_ERROR_FIXED_LIMIT_EXCEEDED	-8
/**
 * \brief Operation timed out
 */
#define GP_ERROR_TIMEOUT                -10

/**
 * \brief Serial ports not supported
 */
#define GP_ERROR_IO_SUPPORTED_SERIAL    -20
/**
 * \brief USB ports not supported
 */
#define GP_ERROR_IO_SUPPORTED_USB       -21

/**
 * \brief Error initialising I/O
 */
#define GP_ERROR_IO_INIT                -31
/**
 * \brief I/O during read
 */
#define GP_ERROR_IO_READ                -34
/**
 * \brief I/O during write
 */
#define GP_ERROR_IO_WRITE               -35
/**
 * \brief I/O during update of settings
 */
#define GP_ERROR_IO_UPDATE              -37

/**
 * \brief Specified serial speed not possible.
 */
#define GP_ERROR_IO_SERIAL_SPEED        -41

/**
 * \brief Error during USB Clear HALT
 */
#define GP_ERROR_IO_USB_CLEAR_HALT      -51
/**
 * \brief Error when trying to find USB device
 */
#define GP_ERROR_IO_USB_FIND            -52
/**
 * \brief Error when trying to claim the USB device
 */
#define GP_ERROR_IO_USB_CLAIM           -53

/**
 * \brief Error when trying to lock the device
 */
#define GP_ERROR_IO_LOCK                -60

/**
 * \brief Unspecified error when talking to HAL
 */
#define GP_ERROR_HAL                    -70


/**
 * \brief Corrupted data received
 *
 * Data is corrupt. This error is reported by camera drivers if corrupted
 * data has been received that can not be automatically handled. Normally,
 * drivers will do everything possible to automatically recover from this
 * error.
 **/
#define GP_ERROR_CORRUPTED_DATA      -102 /* Corrupted data             */

/**
 * \brief File already exists
 *
 * An operation failed because a file existed. This error is reported for
 * example when the user tries to create a file that already exists.
 **/
#define GP_ERROR_FILE_EXISTS         -103

/**
 * \brief Specified camera model was not found
 *
 * The specified model could not be found. This error is reported when
 * the user specified a model that does not seem to be supported by 
 * any driver.
 **/
#define GP_ERROR_MODEL_NOT_FOUND     -105

/**
 * \brief Specified directory was not found
 *
 * The specified directory could not be found. This error is reported when
 * the user specified a directory that is non-existent.
 **/
#define GP_ERROR_DIRECTORY_NOT_FOUND -107

/**
 * \brief Specified file was not found
 *
 * The specified file could not be found. This error is reported when
 * the user wants to access a file that is non-existent.
 **/
#define GP_ERROR_FILE_NOT_FOUND      -108

/**
 * \brief Specified directory already exists
 *
 * The specified directory already exists. This error is reported for example 
 * when the user wants to create a directory that already exists.
 **/
#define GP_ERROR_DIRECTORY_EXISTS    -109

/**
 * \brief The camera is already busy 
 *
 * Camera I/O or a command is in progress.
 **/
#define GP_ERROR_CAMERA_BUSY    -110

/**
 * \brief Path is not absolute
 * 
 * The specified path is not absolute. This error is reported when the user
 * specifies paths that are not absolute, i.e. paths like "path/to/directory".
 * As a rule of thumb, in gphoto2, there is nothing like relative paths.
 **/
#define GP_ERROR_PATH_NOT_ABSOLUTE   -111

/**
 * \brief Cancellation successful.
 *
 * A cancellation requestion by the frontend via progress callback and
 * GP_CONTEXT_FEEDBACK_CANCEL was successful and the transfer has been aborted.
 */
#define GP_ERROR_CANCEL              -112

/**
 * \brief Unspecified camera error
 *
 * The camera reported some kind of error. This can be either a
 * photographic error, such as failure to autofocus, underexposure, or
 * violating storage permission, anything else that stops the camera
 * from performing the operation.
 */
#define GP_ERROR_CAMERA_ERROR	     -113

/**
 * \brief Unspecified failure of the operating system
 *
 * There was some sort of OS error in communicating with the camera,
 * e.g. lack of permission for an operation.
 */
#define GP_ERROR_OS_FAILURE	     -114

/**
 * \brief Not enough space
 *
 * There was not enough free space when uploading a file.
 */
#define GP_ERROR_NO_SPACE	     -115


#endif

#ifndef __MYAPERR__
#define __MYAPERR__
//airphoto 错误码
//#define APERR_UNDEF -501 //未定义错误
//#define APERR_BUFSMALL -506 //预置缓冲区过小
#define AP_ERROR_NOAIRPHOTO -517 //检测不到airphoto
#define AP_ERROR_SOCKET -518 //网络错误
#define AP_ERROR_SDKUNINIT -519 //airphoto sdk未初始化
#define AP_ERROR_PATHTOOLONG -510	//路径过长超出限制
#endif
