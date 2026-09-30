#pragma once

#include <FS.h>
#include <SD_MMC.h>
#include <JPEGDEC.h>

#define jpegOpenSD_MMC(f, m, n) SD_MMC.open(f)
#define jpegClose(f) f.close()
#define jpegRead(f) f.read()
#define jpegSeek(f, n) f.seek(n, SeekSet)
