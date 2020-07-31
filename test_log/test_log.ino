/* Example showing timestamp support in LittleFS */
/* Released into the public domain. */
/* Earle F. Philhower, III <earlephilhower@yahoo.com> */

#include <FS.h>
#include <LittleFS.h>
#include <time.h>
#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "Xiaomi_D6C8"
#define STAPSK  "guyd5161"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

long timezone = 2;
byte daysavetime = 1;


bool getLocalTime(struct tm * info, uint32_t ms) {
  uint32_t count = ms / 10;
  time_t now;

  time(&now);
  localtime_r(&now, info);

  if (info->tm_year > (2016 - 1900)) {
    return true;
  }

  while (count--) {
    delay(10);
    time(&now);
    localtime_r(&now, info);
    if (info->tm_year > (2016 - 1900)) {
      return true;
    }
  }
  return false;
}


void listDir(const char * dirname) {
  Serial.printf("Listing directory: %s\n", dirname);

  Dir root = LittleFS.openDir(dirname);

  while (root.next()) {
    File file = root.openFile("r");
    Serial.print("  FILE: ");
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm * tmstruct = localtime(&cr);
    Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
}


void readFile(const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("Read from file: ");
  int z = 0;
  int b = 0;
  char lines[10][30];
  while (file.available()) {
    char tt = file.read();
    if (tt != '\r') {
      lines[z][b] = tt;
      Serial.print(z);
      Serial.print(",");
      Serial.print(b);
      Serial.print(": ");
      Serial.println(lines[z][b]);
      b++;
    }
    else {
      Serial.print("Line #");
      Serial.print(z);
      Serial.print(":");
      lines[z][b] = '\0';
      Serial.println(lines[z]);
      z++;
      b = 0;
    }
  }
  file.close();
}

void writeFile(const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  delay(2000); // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void appendFile(const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (LittleFS.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (LittleFS.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void setup() {
  Serial.begin(115200);
  // We start by connecting to a WiFi network
  //  Serial.println();
  //  Serial.println();
  //  Serial.print("Connecting to ");
  //  Serial.println(ssid);
  //
  //  WiFi.begin(ssid, pass);
  //
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }
  //  Serial.println("WiFi connected");
  //  Serial.println("IP address: ");
  //  Serial.println(WiFi.localIP());
  //  Serial.println("Contacting Time Server");
  //  configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "pool.ntp.org", "pool.ntp.org");
  //  struct tm tmstruct ;
  //  delay(2000);
  //  tmstruct.tm_year = 0;
  //  getLocalTime(&tmstruct, 5000);
  //  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
  //  Serial.println("");
  //  Serial.println("Formatting LittleFS filesystem");
  //  LittleFS.format();
  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }
  //  listDir("/");
  deleteFile("/hello.txt");
  writeFile("/hello.txt", "avdSDFVSDFVSDFVSDDFVSFefguydvir\r");
  appendFile("/hello.txt", "This is 2nd line\r");
  appendFile("/hello.txt", "This is 3nd line\r");
  appendFile("/hello.txt", "This is 4th line\r");
  //  listDir("/");
  //
  //  Serial.println("The timestamp should be valid above");
  //
  //  Serial.println("Now unmount and remount and perform the same operation.");
  //  Serial.println("Timestamp should be valid, data should be good.");
  LittleFS.end();
  //  Serial.println("Now mount it");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }
  readFile("/hello.txt");
  //  listDir("/");


}

void loop() { }
