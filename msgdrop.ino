
// MsgDrop - Offline Captive Poral Web Browser Based Message Board
// based on: PopupChat https://github.com/tlack/popup-chat
// based on: Captive Portal by: M. Ray Burnette 20150831
// moded tomhiggins
// botched servidge
#include <ESP8266WiFi.h>
#include <DNSServer.h> 
#include <ESP8266WebServer.h>

// User Defined Configuration - Please Change To Suit Your Needs
#define CHATNAME "SpeakFriend"
#define BLURB "Local Msg Board"
#define COMPLAINTSTO "devnull" //unused
#define INDEXTITLE "Hello friend!"
#define INDEXBANNER "This is a local-only msg board that you use through any web browser. Messages are not stored and will not survive at reboot.<a href=/faq>See frequently asked questions..</a>"
#define POSTEDTITLE "Message posted!"
#define POSTEDBANNER "Your message will stay live for a short time or until the server is rebooted."
const String FAQ = "This is an anonymous local-only, non-cloud msg board that only works within the wifi range of a diskless device.<br/>"
"You can not contact the internet from here but you can use this message board and the functions herein.<br/>"
"It is anonymous, only the last few messages are kept, and nothing is saved permanently.<br/><a href=/index>Back..</a>";

// Init System Settings
#define VER "whoever"
#define REBOOTURL "rebootrebootreboot"
#define SHUTDOWNURL "nomorechatnomorechat"
const byte HTTP_CODE = 200; // nyi? 511; // rfc6585
const byte DNS_PORT = 53;  // Capture DNS requests on port 53
const byte TICK_TIMER = 1000;
IPAddress APIP(10, 10, 128, 1);    // Private network for server

// state:
String allMsgs="<i>*system reset and active*</i>";
unsigned long bootTime=0, lastActivity=0, lastTick=0, tickCtr=0; // timers
DNSServer dnsServer; ESP8266WebServer webServer(80); // standard api servers

String input(String argName) {
  String a=webServer.arg(argName);
  a.replace("<","&lt;");a.replace(">","&gt;");
  a.substring(0,200); return a; }

String footer() {
  return
  "<div class=by>" VER "</div></body></html>"; }

String header(String t) {
  unsigned char softap_stations_cnt;
  softap_stations_cnt = wifi_softap_get_station_num(); 
  String a = String(CHATNAME);
  String CSS = "article { background: #f2f2f2; padding: 1em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 12px; line-height: 15px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0; }"
    "input { border-radius: 0; }"
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "nav { background: #0000FF; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.2em; margin-bottom: 0.5em; } "
    "ol { padding-left: 20px; padding-right: 0px; width: 100%; }"
    "input[type='textinput'] { width: 100%; }"
    "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
    "<head><title>"+a+" :: "+t+"</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><nav><b>"+a+"</b><div1 style=\"text-align:left; float:left;\">"+BLURB+"</div1><div style=\"text-align:right;\"> Active Clients: "+softap_stations_cnt+"</div></nav><div><h1>"+t+"</h1></div><div>";
  return h; }

String faq() {
  return header("frequently asked questions") + FAQ + footer();
}

String index() {
  return header(INDEXTITLE) + "<div>" + INDEXBANNER + "</div><div><label>Messages (Nr.# Timestamp_Message):</label><ol>"+allMsgs+
  "</ol></div>"
  "<div><form class='textarea' id='textarea' action=/post method=post></div>"+
  "<div1 style=\"text-align:left; float:left; padding:0px;\"><label>Post new message:</label><i>remember:</i> do not use names/ids </div1><div style=\"text-align:right;padding:0px;\"><p id=\"charNum\">160 / 160</p></div>"+
"<script>"
"function countChar(obj){"
    "var maxLength = 160;"
    "var strLength = obj.value.length;"
    "var charRemain = (maxLength - strLength);"
        "document.getElementById(\"charNum\").innerHTML = charRemain+' / 160';"
        //"document.getElementById(\"zeichen\").value = charRemain+' / 160';"
"}</script>"
"<input type='textinput' id='textinput' name='m' maxlength='160' autocomplete=\"off\" onkeyup='countChar(this);'/>"
"<BR><input type='submit' value='Send'/></form>"
"<br/>" + footer();  
}


String posted() {
unsigned long t=millis()/1000;
static char timeStamp[12];
 long h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 int s = t % 60;
 sprintf(timeStamp, "%04ld:%02d:%02d", h, m, s);
  String msg=input("m"); allMsgs=allMsgs+"<li>"+timeStamp+"_"+msg+"</li>";
  return header(POSTEDTITLE) + POSTEDBANNER + "<article>"+msg+"</article><a href=/>Back to Msg Board</a>" + footer();
}

String reboot() {
  return header("Device will now be restarted.") + footer();
}
String shutdown() {
  return header("Device will be shut down till Power Cycle.") + footer();
}

void esprestart(){  
  delay(3000);
  ESP.restart(); 
}

void espshutdown(){
  delay(3000);
  ESP.deepSleep(0); 
}

void setup() {
  
  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 128, 0));
  WiFi.softAP(CHATNAME);
  dnsServer.start(DNS_PORT, "*", APIP);
  webServer.on("/post",[]() { webServer.send(HTTP_CODE, "text/html", posted()); });
  webServer.on("/faq",[]() { webServer.send(HTTP_CODE, "text/html", faq()); });
  webServer.on("/"REBOOTURL,[]() { webServer.send(HTTP_CODE, "text/html", reboot()); esprestart(); });
  webServer.on("/"SHUTDOWNURL,[]() { webServer.send(HTTP_CODE, "text/html", shutdown()); espshutdown(); });
  webServer.onNotFound([]() { lastActivity=millis(); webServer.send(HTTP_CODE, "text/html", index()); });
  webServer.begin();
}


void loop() { 
  if ((millis()-lastTick)>TICK_TIMER) {lastTick=millis();} 
dnsServer.processNextRequest(); webServer.handleClient(); }
