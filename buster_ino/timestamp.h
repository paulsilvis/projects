#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

typedef struct TimeStamp {
  unsigned char second;
  unsigned char minute;
  unsigned char hour;
  unsigned char dayOfWeek;
  unsigned char dayOfMonth;
  unsigned char month;
  unsigned char year;
} TimeStamp;

typedef struct ShotRecord {
  unsigned char numEntries;
  TimeStamp entries[10];
} ShotRecord;

#endif

