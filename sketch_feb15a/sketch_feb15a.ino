#include <Arduino.h>
#define _EOL '\r'

String a = "";
void emptyBuffer()
{
  a = "";
}
void insert_record_to_buffer(const char *inmsg)
{
  a += String(inmsg);
  a += _EOL;
}
int getBuffer_records()
{
  bool x = true;
  int _end = 0;
  int _start = 0;
  int lineCounter = 0;

  while (x == true)
  {
    _end = a.indexOf(_EOL, _start);
    _start = _end + 1;
    if (_end == a.length() - 1)
    {
      return lineCounter;
    }
    lineCounter++;
  }
}
String getLine(int requested_line)
{
  bool b = true;
  int _start = 0;
  int _end = 0;
  int lineCounter = 0;

  while (lineCounter <= requested_line)
  {
    _end = a.indexOf(_EOL, _start);
    if (lineCounter == requested_line)
    {
      return a.substring(_start, _end);
    }
    _start = _end + 1;
    if (_end == a.length() - 1)
    {
      return "Error";
    }
    lineCounter++;
  }
}

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(100);

  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("MJYJ&^&YHT%BR");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("MJYJ&^&YHT%BR");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("MJYJ&^&YHT%BR");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("MJYJ&^&YHT%BR");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("zzzzzzz");
  insert_record_to_buffer("ABCD");
  insert_record_to_buffer("1234");
  insert_record_to_buffer("FVWFWFVWFVWF");
  insert_record_to_buffer("fevfrvedce");
  insert_record_to_buffer("fevfsgblsitbjw98yh6u256yhwrtphgwsoitjhwlrotihjwpotihjs;flgkbjgwrtoyhi923y6howjrthosfgtdhnj;wlrthyjw64poihyt;glhjikswthpow4u6yp5hwtrhvedce");
  insert_record_to_buffer("XCVD");
  insert_record_to_buffer("123");
  insert_record_to_buffer("zzzzBRGBTYNBTBTC");
  insert_record_to_buffer("zzzzzzz");

  Serial.println("\nStart!");
  Serial.print("total length: ");
  Serial.println(a.length());
  Serial.print("total lines: ");
  Serial.println(getBuffer_records());

  // Serial.println(getLine(a));

  for (int aa = 0; aa <= getBuffer_records(); aa++)
  {
    Serial.print("Line #");
    Serial.print(aa);
    Serial.print(" >> ");
    Serial.println(getLine(aa));
  }
  Serial.println("<<THE-END>>");
}

void loop()
{
  delay(1000);
}
