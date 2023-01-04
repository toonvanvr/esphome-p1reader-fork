//-------------------------------------------------------------------------------------
// ESPHome P1 Electricity Meter custom sensor
// Copyright 2020 Pär Svanström
//
// History
//  0.1.0 2020-11-05:   Initial release
//
// MIT License
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-------------------------------------------------------------------------------------

#include "esphome.h"

#define BUF_SIZE 60

class ParsedMessage
{
public:
  double eActiveIn;       // derived
  double eActiveInDay;    // 1.8.1
  double eActiveInNight;  // 1.8.2
  double eActiveOut;      // derived
  double eActiveOutDay;   // 2.8.1
  double eActiveOutNight; // 2.8.2
  double pIn;             // 1.7.0
  double pOut;            // 2.7.0
  double u1;              // 32.7.0
  double u2;              // 52.7.0
  double u3;              // 72.7.0
  double i1;              // 31.7.0
  double i2;              // 51.7.0
  double i3;              // 71.7.0
  double p1In;            // 21.7.0
  double p2In;            // 41.7.0
  double p3In;            // 61.7.0
  double p1Out;           // 22.7.0
  double p2Out;           // 42.7.0
  double p3Out;           // 62.7.0

  bool crcOk = false;
};

class P1Reader : public Component, public UARTDevice
{
  const char *DELIMITERS = "()*:";
  const char *DATA_ID = "1-0";
  char buffer[BUF_SIZE];

public:
  Sensor *eActiveIn = new Sensor();
  Sensor *eActiveInDay = new Sensor();
  Sensor *eActiveInNight = new Sensor();
  Sensor *eActiveOut = new Sensor();
  Sensor *eActiveOutDay = new Sensor();
  Sensor *eActiveOutNight = new Sensor();
  Sensor *pIn = new Sensor();
  Sensor *pOut = new Sensor();
  Sensor *u1 = new Sensor();
  Sensor *u2 = new Sensor();
  Sensor *u3 = new Sensor();
  Sensor *i1 = new Sensor();
  Sensor *i2 = new Sensor();
  Sensor *i3 = new Sensor();
  Sensor *p1In = new Sensor();
  Sensor *p2In = new Sensor();
  Sensor *p3In = new Sensor();
  Sensor *p1Out = new Sensor();
  Sensor *p2Out = new Sensor();
  Sensor *p3Out = new Sensor();

  P1Reader(UARTComponent *parent) : UARTDevice(parent) {}

  void setup() override {}

  void loop() override
  {
    readP1Message();
  }

private:
  uint16_t crc16_update(uint16_t crc, uint8_t a)
  {
    int i;
    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ 0xA001;
      }
      else
      {
        crc = (crc >> 1);
      }
    }
    return crc;
  }

  void parseRow(ParsedMessage *parsed, char *obisCode, char *value)
  {
    if (strncmp(obisCode, "1.8.1", 5) == 0)
    {
      parsed->eActiveInDay = atof(value);
    }
    else if (strncmp(obisCode, "1.8.2", 5) == 0)
    {
      parsed->eActiveInNight = atof(value);
    }
    else if (strncmp(obisCode, "2.8.1", 5) == 0)
    {
      parsed->eActiveOutDay = atof(value);
    }
    else if (strncmp(obisCode, "2.8.2", 5) == 0)
    {
      parsed->eActiveOutNight = atof(value);
    }
    else if (strncmp(obisCode, "1.7.0", 5) == 0)
    {
      parsed->pIn = atof(value);
    }
    else if (strncmp(obisCode, "2.7.0", 5) == 0)
    {
      parsed->pOut = atof(value);
    }
    else if (strncmp(obisCode, "32.7.0", 6) == 0)
    {
      parsed->u1 = atof(value);
    }
    else if (strncmp(obisCode, "52.7.0", 6) == 0)
    {
      parsed->u2 = atof(value);
    }
    else if (strncmp(obisCode, "72.7.0", 6) == 0)
    {
      parsed->u3 = atof(value);
    }
    else if (strncmp(obisCode, "31.7.0", 6) == 0)
    {
      parsed->i1 = atof(value);
    }
    else if (strncmp(obisCode, "51.7.0", 6) == 0)
    {
      parsed->i2 = atof(value);
    }
    else if (strncmp(obisCode, "71.7.0", 6) == 0)
    {
      parsed->i3 = atof(value);
    }
    else if (strncmp(obisCode, "21.7.0", 6) == 0)
    {
      parsed->p1In = atof(value);
    }
    else if (strncmp(obisCode, "41.7.0", 6) == 0)
    {
      parsed->p2In = atof(value);
    }
    else if (strncmp(obisCode, "61.7.0", 6) == 0)
    {
      parsed->p3In = atof(value);
    }
    else if (strncmp(obisCode, "22.7.0", 6) == 0)
    {
      parsed->p1Out = atof(value);
    }
    else if (strncmp(obisCode, "42.7.0", 6) == 0)
    {
      parsed->p2Out = atof(value);
    }
    else if (strncmp(obisCode, "62.7.0", 6) == 0)
    {
      parsed->p3Out = atof(value);
    }
  }

  void publishSensors(ParsedMessage *parsed)
  {
    eActiveIn->publish_state(parsed->eActiveIn);
    eActiveInDay->publish_state(parsed->eActiveInDay);
    eActiveInNight->publish_state(parsed->eActiveInNight);
    eActiveOut->publish_state(parsed->eActiveOut);
    eActiveOutDay->publish_state(parsed->eActiveOutDay);
    eActiveOutNight->publish_state(parsed->eActiveOutNight);
    pIn->publish_state(parsed->pIn);
    pOut->publish_state(parsed->pOut);
    u1->publish_state(parsed->u1);
    u2->publish_state(parsed->u2);
    u3->publish_state(parsed->u3);
    i1->publish_state(parsed->i1);
    i2->publish_state(parsed->i2);
    i3->publish_state(parsed->i3);
    p1In->publish_state(parsed->p1In);
    p2In->publish_state(parsed->p2In);
    p3In->publish_state(parsed->p3In);
    p1Out->publish_state(parsed->p1Out);
    p2Out->publish_state(parsed->p2Out);
    p3Out->publish_state(parsed->p3Out);
  }

  void readP1Message()
  {
    if (available())
    {
      uint16_t crc = 0x0000;
      ParsedMessage parsed = ParsedMessage();
      bool telegramEnded = false;

      while (available())
      {
        int len = Serial.readBytesUntil('\n', buffer, BUF_SIZE);

        if (len > 0)
        {
          ESP_LOGD("data", "%s", buffer);

          // put newline back as it is required for CRC calculation
          buffer[len] = '\n';
          buffer[len + 1] = '\0';

          // if we've reached the CRC checksum, calculate last CRC and compare
          if (buffer[0] == '!')
          {
            crc = crc16_update(crc, buffer[0]);
            int crcFromMsg = (int)strtol(&buffer[1], NULL, 16);
            parsed.crcOk = crc == crcFromMsg;
            ESP_LOGI("crc", "Telegram read. CRC: %04X = %04X. PASS = %s", crc, crcFromMsg, parsed.crcOk ? "YES" : "NO");
            telegramEnded = true;

            // otherwise pass the row through the CRC calculation
          }
          else
          {
            for (int i = 0; i < len + 1; i++)
            {
              crc = crc16_update(crc, buffer[i]);
            }
          }

          // if this is a row containing information
          if (strchr(buffer, '(') != NULL)
          {
            char *dataId = strtok(buffer, DELIMITERS);
            char *obisCode = strtok(NULL, DELIMITERS);

            // ...and this row is a data row, then parse row
            if (strncmp(DATA_ID, dataId, strlen(DATA_ID)) == 0)
            {
              char *value = strtok(NULL, DELIMITERS);
              char *unit = strtok(NULL, DELIMITERS);
              parseRow(&parsed, obisCode, value);
            }
          }
        }
        // clean buffer
        memset(buffer, 0, BUF_SIZE - 1);

        if (!telegramEnded && !available())
        {
          // wait for more data
          delay(40);
        }
      }

      // if the CRC pass, publish sensor values
      if (parsed.crcOk)
      {
        parsed.eActiveIn = parsed.eActiveInDay + parsed.eActiveInNight;
        parsed.eActiveOut = parsed.eActiveOutDay + parsed.eActiveOutNight;
        publishSensors(&parsed);
      }
      else
      {
        ESP_LOGD("data", "CRC Check failed");
      }
    }
  }
};
