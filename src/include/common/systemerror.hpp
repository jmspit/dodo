/*
 * This file is part of the dodo library (https://github.com/jmspit/dodo).
 * Copyright (c) 2019 Jan-Marten Spit.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file systemerror.hpp
 * Defines the dodo::common::SystemError class.
 */

#ifndef common_systemerror_hpp
#define common_systemerror_hpp


#include <cstring>
#include <netdb.h>
#include <sstream>
#include <string>

#include "common/puts.hpp"

namespace dodo::common {

  using namespace std;

    /**
     * Linux system error primitive to provide a consistent interface to
     * Linux error codes. The [[nodiscard]] attribute causes the compiler to issue a warning in case
     * a SystemError is produced as rvalue but not assigned to an lvalue - encouraging more robust
     * code by pinpointing omitted error checking.
     * @code
     * SystemError error1; // set to ecOK
     * SystemError error2( SystemError::ecECONNREFUSED ); // set to ECONNREFUSED
     * error1 == error2; // false
     * error1 != error2; // true
     * error1 == SystemError::ecECONNREFUSED; // false
     * error2 == SystemError::ecECONNREFUSED; // true
     * error2 == SystemError( SystemError::ecECONNREFUSED ); //true
     * int i = error1; // error1 cast to int=ecOK
     * error1 = SystemError::ecEBADF // error1 set to SystemError::ecEBADF
     * @endcode
     */
    class [[nodiscard]] SystemError {
      public:

        /**
         * Enumarate mimicing Linux error codes, integrates the EAI (addrinfo) ranges of errors.
         */
        enum ErrorCode {
          ecOK = 0,                                /**< 0   Not an error, success */
          ecEPERM = EPERM,                        /**< 1   Operation not permitted  */
          ecENOENT = ENOENT,                      /**< 2   No such file or directory  */
          ecESRCH = ESRCH,                        /**< 3   No such process  */
          ecEINTR = EINTR,                        /**< 4   Interrupted system call  */
          ecEIO = EIO,                            /**< 5   I/O error  */
          ecENXIO = ENXIO,                        /**< 6   No such device or address  */
          ecE2BIG = E2BIG,                        /**< 7   Argument list too long  */
          ecENOEXEC = ENOEXEC,                    /**< 8   Exec format error  */
          ecEBADF = EBADF,                        /**< 9   Bad file number  */
          ecECHILD = ECHILD,                      /**< 10   No child processes  */
          ecEAGAIN = EAGAIN,                      /**< 11   Try again  */
          ecENOMEM = ENOMEM,                      /**< 12   Out of memory  */
          ecEACCES = EACCES,                      /**< 13   Permission denied  */
          ecEFAULT = EFAULT,                      /**< 14   Bad address  */
          ecENOTBLK = ENOTBLK,                    /**< 15   Block device required  */
          ecEBUSY = EBUSY,                        /**< 16   Device or resource busy  */
          ecEEXIST = EEXIST,                      /**< 17   File exists  */
          ecEXDEV = EXDEV,                        /**< 18   Cross-device link  */
          ecENODEV = ENODEV,                      /**< 19   No such device  */
          ecENOTDIR = ENOTDIR,                    /**< 20   Not a directory  */
          ecEISDIR = EISDIR,                      /**< 21   Is a directory  */
          ecEINVAL = EINVAL,                      /**< 22   Invalid argument  */
          ecENFILE = ENFILE,                      /**< 23   File table overflow  */
          ecEMFILE = EMFILE,                      /**< 24   Too many open files  */
          ecENOTTY = ENOTTY,                      /**< 25   Not a typewriter  */
          ecETXTBSY = ETXTBSY,                    /**< 26   Text file busy  */
          ecEFBIG = EFBIG,                        /**< 27   File too large  */
          ecENOSPC = ENOSPC,                      /**< 28   No space left on device  */
          ecESPIPE = ESPIPE,                      /**< 29   Illegal seek  */
          ecEROFS = EROFS,                        /**< 30   Read-only file system  */
          ecEMLINK = EMLINK,                      /**< 31   Too many links  */
          ecEPIPE = EPIPE,                        /**< 32   Broken pipe  */
          ecEDOM = EDOM,                          /**< 33   Math argument out of domain of func  */
          ecERANGE = ERANGE,                      /**< 34   Math result not representable  */
          ecEDEADLK = EDEADLK,                    /**< 35   Resource deadlock would occur  */
          ecENAMETOOLONG = ENAMETOOLONG,          /**< 36   File name too long  */
          ecENOLCK = ENOLCK,                      /**< 37   No record locks available  */
          ecENOSYS = ENOSYS,                      /**< 38   Invalid system call number  */
          ecENOTEMPTY = ENOTEMPTY,                /**< 39   Directory not empty  */
          ecELOOP = ELOOP,                        /**< 40   Too many symbolic links encountered  */
          ecEWOULDBLOCK = EWOULDBLOCK,            /**< 41   Operation would block  */
          ecENOMSG = ENOMSG,                      /**< 42   No message of desired type  */
          ecEIDRM = EIDRM,                        /**< 43   Identifier removed  */
          ecECHRNG = ECHRNG,                      /**< 44   Channel number out of range  */
          ecEL2NSYNC = EL2NSYNC,                  /**< 45   Level 2 not synchronized  */
          ecEL3HLT = EL3HLT,                      /**< 46   Level 3 halted  */
          ecEL3RST = EL3RST,                      /**< 47   Level 3 reset  */
          ecELNRNG = ELNRNG,                      /**< 48   Link number out of range  */
          ecEUNATCH = EUNATCH,                    /**< 49   Protocol driver not attached  */
          ecENOCSI = ENOCSI,                      /**< 50   No CSI structure available  */
          ecEL2HLT = EL2HLT,                      /**< 51   Level 2 halted  */
          ecEBADE = EBADE,                        /**< 52   Invalid exchange  */
          ecEBADR = EBADR,                        /**< 53   Invalid request descriptor  */
          ecEXFULL = EXFULL,                      /**< 54   Exchange full  */
          ecENOANO = ENOANO,                      /**< 55   No anode  */
          ecEBADRQC = EBADRQC,                    /**< 56   Invalid request code  */
          ecEBADSLT = EBADSLT,                    /**< 57   Invalid slot  */
          ecEDEADLOCK = EDEADLOCK,                /**< 58   EDEADLK */
          ecEBFONT = EBFONT,                      /**< 59   Bad font file format  */
          ecENOSTR = ENOSTR,                      /**< 60   Device not a stream  */
          ecENODATA = ENODATA,                    /**< 61   No data available  */
          ecETIME = ETIME,                        /**< 62   Timer expired  */
          ecENOSR = ENOSR,                        /**< 63   Out of streams resources  */
          ecENONET = ENONET,                      /**< 64   Machine is not on the network  */
          ecENOPKG = ENOPKG,                      /**< 65   Package not installed  */
          ecEREMOTE = EREMOTE,                    /**< 66   Object is remote  */
          ecENOLINK = ENOLINK,                    /**< 67   Link has been severed  */
          ecEADV = EADV,                          /**< 68   Advertise error  */
          ecESRMNT = ESRMNT,                      /**< 69   Srmount error  */
          ecECOMM = ECOMM,                        /**< 70   Communication error on send  */
          ecEPROTO = EPROTO,                      /**< 71   Protocol error  */
          ecEMULTIHOP = EMULTIHOP,                /**< 72   Multihop attempted  */
          ecEDOTDOT = EDOTDOT,                    /**< 73   RFS specific error  */
          ecEBADMSG = EBADMSG,                    /**< 74   Not a data message  */
          ecEOVERFLOW = EOVERFLOW,                /**< 75   Value too large for defined data type  */
          ecENOTUNIQ = ENOTUNIQ,                  /**< 76   Name not unique on network  */
          ecEBADFD = EBADFD,                      /**< 77   File descriptor in bad state  */
          ecEREMCHG = EREMCHG,                    /**< 78   Remote address changed  */
          ecELIBACC = ELIBACC,                    /**< 79   Can not access a needed shared library  */
          ecELIBBAD = ELIBBAD,                    /**< 80   Accessing a corrupted shared library  */
          ecELIBSCN = ELIBSCN,                    /**< 81   .lib section in a.out corrupted  */
          ecELIBMAX = ELIBMAX,                    /**< 82   Attempting to link in too many shared libraries  */
          ecELIBEXEC = ELIBEXEC,                  /**< 83   Cannot exec a shared library directly  */
          ecEILSEQ = EILSEQ,                      /**< 84   Illegal byte sequence  */
          ecERESTART = ERESTART,                  /**< 85   Interrupted system call should be restarted  */
          ecESTRPIPE = ESTRPIPE,                  /**< 86   Streams pipe error  */
          ecEUSERS = EUSERS,                      /**< 87   Too many users  */
          ecENOTSOCK = ENOTSOCK,                  /**< 88   Socket operation on non-socket  */
          ecEDESTADDRREQ = EDESTADDRREQ,          /**< 89   Destination address required  */
          ecEMSGSIZE = EMSGSIZE,                  /**< 90   Message too long  */
          ecEPROTOTYPE = EPROTOTYPE,              /**< 91   Protocol wrong type for socket  */
          ecENOPROTOOPT = ENOPROTOOPT,            /**< 92   Protocol not available  */
          ecEPROTONOSUPPORT = EPROTONOSUPPORT,    /**< 93   Protocol not supported  */
          ecESOCKTNOSUPPORT = ESOCKTNOSUPPORT,    /**< 94   Socket type not supported  */
          ecEOPNOTSUPP = EOPNOTSUPP,              /**< 95   Operation not supported on transport endpoint  */
          ecEPFNOSUPPORT = EPFNOSUPPORT,          /**< 96   Protocol family not supported  */
          ecEAFNOSUPPORT = EAFNOSUPPORT,          /**< 97   Address family not supported by protocol  */
          ecEADDRINUSE = EADDRINUSE,              /**< 98   Address already in use  */
          ecEADDRNOTAVAIL = EADDRNOTAVAIL,        /**< 99   Cannot assign requested address  */
          ecENETDOWN = ENETDOWN,                  /**< 100  Network is down  */
          ecENETUNREACH = ENETUNREACH,            /**< 101  Network is unreachable  */
          ecENETRESET = ENETRESET,                /**< 102  Network dropped connection because of reset  */
          ecECONNABORTED = ECONNABORTED,          /**< 103  Software caused connection abort  */
          ecECONNRESET = ECONNRESET,              /**< 104  Connection reset by peer  */
          ecENOBUFS = ENOBUFS,                    /**< 105  No buffer space available  */
          ecEISCONN = EISCONN,                    /**< 106  Transport endpoint is already connected  */
          ecENOTCONN = ENOTCONN,                  /**< 107  Transport endpoint is not connected  */
          ecESHUTDOWN = ESHUTDOWN,                /**< 108  Cannot send after transport endpoint shutdown  */
          ecETOOMANYREFS = ETOOMANYREFS,          /**< 109  Too many references: cannot splice  */
          ecETIMEDOUT = ETIMEDOUT,                /**< 110  Connection timed out  */
          ecECONNREFUSED = ECONNREFUSED,          /**< 111  Connection refused  */
          ecEHOSTDOWN = EHOSTDOWN,                /**< 112  Host is down  */
          ecEHOSTUNREACH = EHOSTUNREACH,          /**< 113  No route to host  */
          ecEALREADY = EALREADY,                  /**< 114  Operation already in progress  */
          ecEINPROGRESS = EINPROGRESS,            /**< 115  Operation now in progress  */
          ecESTALE = ESTALE,                      /**< 116  Stale file handle  */
          ecEUCLEAN = EUCLEAN,                    /**< 117  Structure needs cleaning  */
          ecENOTNAM = ENOTNAM,                    /**< 118  Not a XENIX named type file  */
          ecENAVAIL = ENAVAIL,                    /**< 119  No XENIX semaphores available  */
          ecEISNAM = EISNAM,                      /**< 120  Is a named type file  */
          ecEREMOTEIO = EREMOTEIO,                /**< 121  Remote I/O error  */
          ecEDQUOT = EDQUOT,                      /**< 122  Quota exceeded  */
          ecENOMEDIUM = ENOMEDIUM,                /**< 123  No medium found  */
          ecEMEDIUMTYPE = EMEDIUMTYPE,            /**< 124  Wrong medium type  */
          ecECANCELED = ECANCELED,                /**< 125  Operation Canceled  */
          ecENOKEY = ENOKEY,                      /**< 126  Required key not available  */
          ecEKEYEXPIRED = EKEYEXPIRED,            /**< 127  Key has expired  */
          ecEKEYREVOKED = EKEYREVOKED,            /**< 128  Key has been revoked  */
          ecEKEYREJECTED = EKEYREJECTED,          /**< 129  Key was rejected by service  */
          ecEOWNERDEAD = EOWNERDEAD,              /**< 130  Owner died  */
          ecENOTRECOVERABLE = ENOTRECOVERABLE,    /**< 131  State not recoverable  */
          ecERFKILL = ERFKILL,                    /**< 132  Operation not possible due to RF-kill  */
          ecEHWPOISON = EHWPOISON,                /**< 133  Memory page has hardware error  */


          ecEAI_BADFLAGS = EAI_BADFLAGS,          /**< -1   Invalid value for `ai_flags' field.   */
          ecEAI_NONAME = EAI_NONAME,              /**< -2   NAME or SERVICE is unknown.   */
          ecEAI_AGAIN = EAI_AGAIN,                /**< -3   Temporary failure in name resolution.   */
          ecEAI_FAIL = EAI_FAIL,                  /**< -4   Non-recoverable failure in name res.   */
          ecEAI_NODATA = EAI_NODATA,              /**< -5   No address associated with NAME.   */
          ecEAI_FAMILY = EAI_FAMILY,              /**< -6   ai_family not supported.   */
          ecEAI_SOCKTYPE = EAI_SOCKTYPE,          /**< -7   ai_socktype not supported.   */
          ecEAI_SERVICE = EAI_SERVICE,            /**< -8   SERVICE not supported for `ai_socktype'.   */
          ecEAI_ADDRFAMILY = EAI_ADDRFAMILY,      /**< -9   Address family for NAME not supported.   */
          ecEAI_MEMORY = EAI_MEMORY,              /**< -10  Memory allocation failure.   */
          ecEAI_SYSTEM = EAI_SYSTEM,              /**< -11  System error returned in `errno'.   */
          ecEAI_OVERFLOW = EAI_OVERFLOW,          /**< -12  Argument buffer overflow.   */
          ecEAI_INPROGRESS = EAI_INPROGRESS,      /**< -100 Processing request in progress.   */
          ecEAI_CANCELED = EAI_CANCELED,          /**< -101 Request canceled.   */
          ecEAI_NOTCANCELED = EAI_NOTCANCELED,    /**< -102 Request not canceled.   */
          ecEAI_ALLDONE = EAI_ALLDONE,            /**< -103 All requests done.   */
          ecEAI_INTR = EAI_INTR,                  /**< -104 Interrupted by a signal.   */
          ecEAI_IDN_ENCODE = EAI_IDN_ENCODE,      /**< -105 IDN encoding failed.   */

          ecLIBRARY_FIRST = 10000,                    /**< 10000 Marker, library error codes from here. */

          ecSSL = 10002,                              /**< 10002 SSL exception */
          ecSSL_ERROR_NONE = 10003,                   /**< 10003 SSL_ERROR_NONE */
          ecSSL_ERROR_ZERO_RETURN = 10003,            /**< 10003 SSL_ERROR_ZERO_RETURN */
          ecSSL_ERROR_WANT_READ = 10004,              /**< 10004 SSL_ERROR_WANT_READ */
          ecSSL_ERROR_WANT_WRITE = 10005,             /**< 10005 SSL_ERROR_WANT_WRITE */
          ecSSL_ERROR_WANT_CONNECT = 10006,           /**< 10006 SSL_ERROR_WANT_CONNECT */
          ecSSL_ERROR_WANT_ACCEPT = 10007,            /**< 10007 SSL_ERROR_WANT_ACCEPT */
          ecSSL_ERROR_WANT_X509_LOOKUP = 10008,       /**< 10008 SSL_ERROR_WANT_X509_LOOKUP */
          ecSSL_ERROR_WANT_ASYNC = 10009,             /**< 10009 SSL_ERROR_WANT_ASYNC */
          ecSSL_ERROR_WANT_ASYNC_JOB = 10010,         /**< 10010 SSL_ERROR_WANT_ASYNC_JOB */
          ecSSL_ERROR_WANT_CLIENT_HELLO_CB = 10011,   /**< 10011 SSL_ERROR_WANT_CLIENT_HELLO_CB */
          ecSSL_ERROR_SYSCALL = 10012,                /**< 10012 SSL_ERROR_SYSCALL */
          ecSSL_ERROR_PEERVERIFICATION = 10013,       /**< 10013 When peer verification failed */

        };

        /**
         * Construct default with ecOK.
         */
        SystemError() : errorcode_(ecOK) {};

        /**
         * Construct from ErrorCode enum.
         * @param e The ErrorCode to assign.
         */
        SystemError( ErrorCode e ) : errorcode_(e) {};

        /**
         * Construct from system error code.
         * @param e The int error to assign.
         */
        SystemError( int e ) : errorcode_(ErrorCode(e)) {};

        /**
         * Compare this SystemError to the system error e.
         * @param e The int to compare to.
         * @return true when unequal.
         */
        bool operator!=( int e ) { return this->errorcode_ != ErrorCode(e); };

        /**
         * Compare this SystemError to the ErrorCode e.
         * @param e The ErrorCode to compare to.
         * @return true when unequal.
         */
        bool operator!=( ErrorCode e ) { return this->errorcode_ != e; };

        /**
         * Compare this SystemError to the SystemError e.
         * @param e The SystemError to compare to.
         * @return true when unequal.
         */
        bool operator!=( SystemError e ) { return this->errorcode_ != e.errorcode_; };

        /**
         * Compare this SystemError to the ErrorCode e.
         * @param e The error to compare to.
         * @return true when equal.
         */
        bool operator==( ErrorCode e ) { return this->errorcode_ == e; };

        /**
         * Compare this SystemError to the SystemError e.
         * @param e The SystemError to compare to.
         * @return true when equal.
         */
        bool operator==( const SystemError& e ) { return this->errorcode_ == e.errorcode_; };

        /**
         * Assign system error e.
         * @param e The int error to assign
         * @return This SystemError
         */
        SystemError& operator=( int e ) { this->errorcode_ = ErrorCode(e); return *this; };

        /**
         * Assign ErrorCode e.
         * @param e The ErrorCode to assign
         * @return This SystemError
         */
        SystemError& operator=( ErrorCode e ) { this->errorcode_ = e; return *this; };

        /**
         * Assign SystemError e.
         * @param e The SystemError to assign
         * @return This SystemError
         */
        SystemError& operator=( SystemError e ) { this->errorcode_ = e.errorcode_; return *this; };

        /**
         * Cast this SystemError to an int by taking errorcode_.
         * @return The int cast to.
         */
        operator int() const { return errorcode_; };

        /**
         * Get the system error string.
         * @return The system error string.
         */
        string asString() const {
          stringstream ss;
          if ( errorcode_ > 0 ) {
            if ( errorcode_ >= ecLIBRARY_FIRST )
              ss << libstrerror( errorcode_ );
            else
              ss << strerror(errorcode_);
          } else ss << gai_strerror(errorcode_);
          ss << " (" << errorcode_ << ")";
          return ss.str();
        };

        /**
         * Translate SystemErrors in library range.
         * @param error The SystemError to translate.
         * @return The error string.
         */
        static string libstrerror( SystemError error ) {
          switch ( error ) {
            case SystemError::ecSSL_ERROR_PEERVERIFICATION:
              return "The peer certificate CN or SubjectAltNames do not match";
            case SystemError::ecSSL:
              return "SSL exception thrown";
            default : return common::Puts() << "unknown error" << error;
          }
        }

      private:
        /**
         * Enumarate representation of the system error code.
         */
        ErrorCode errorcode_;
  };

}


#endif
