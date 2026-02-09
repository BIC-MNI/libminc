/* ----------------------------- MNI Header -----------------------------------
@NAME       :
@DESCRIPTION: minc exceptions
@COPYRIGHT  :
              Copyright 2006 Vladimir Fonov, McConnell Brain Imaging Centre,
              Montreal Neurological Institute, McGill University.
              Permission to use, copy, modify, and distribute this
              software and its documentation for any purpose and without
              fee is hereby granted, provided that the above copyright
              notice appear in all copies.  The author and McGill University
              make no representations about the suitability of this
              software for any purpose.  It is provided "as is" without
              express or implied warranty.
---------------------------------------------------------------------------- */
#ifndef MINC_IO_EXCEPTIONS_H
#define MINC_IO_EXCEPTIONS_H

#define REPORT_ERROR(MSG) throw minc::generic_error(__FILE__,__LINE__,MSG)
#define CHECK_MINC_CALL(var) { if((var)<0) REPORT_ERROR("MINC CALL FAILED");}


namespace minc
{
  class generic_error
  {
  public:
    const char *_file;
    int _line;
    const char *_msg;
    int _code;
  public:

    generic_error (const char *file__, int line__, const char *msg__ = "Error", int code__ = 0):
    _file (file__), _line (line__), _msg (msg__), _code (code__)
    {
      //                    std::cerr<<"Exception created: "<<_file<<":"<<_line<<" "<<_msg<<std::endl;
    }

    const char *file (void) const
    {
      return _file;
    }

    const char *msg (void) const
    {
      return _msg;
    }

    int line (void) const
    {
      return _line;
    }

    int code (void) const
    {
      return _code;
    }
  };
} //minc
#endif //MINC_IO_EXCEPTIONS_H
