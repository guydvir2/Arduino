const char *onflash_Paramter[] = {"gulkjhlkjhlkjhlkjhlkjly", "dvir", "Anna", "OZ", "SSSSSSS"};
const char *fileList[4];
int fileCount=0;
void processFiles(const char *_fileList[], int _fileCount)
{
  fileCount=_fileCount;
  for (int i = 0; i < fileCount; ++i) {
      Serial.print("Processing file ");
      fileList[i]=_fileList[i];
      Serial.println(fileList[i]);

  }
}

// char *y="GUY";
void setup()
{
  Serial.begin(115200);
  Serial.println("\nStart");
  processFiles(onflash_Paramter,4);

}

void loop()
{
}
