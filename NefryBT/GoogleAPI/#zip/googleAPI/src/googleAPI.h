#ifndef GOOGLEAPI_H
#define GOOGLEAPI_H
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define NEFRY_DATASTORE_BUCKET 4
#define NEFRY_DATASTORE_REFRESH_TOKEN 5
#define NEFRY_DATASTORE_CLIENT_ID 6
#define NEFRY_DATASTORE_CLIENT_SECRET 7
#define NEFRY_DATASTORE_PARENT 8

class googleAPI
{
  String token_uri = "/oauth2/v4/token";
  String drive_uri = "/upload/drive/v3/files?uploadType=multipart";
  String storage_uri = "/upload/storage/v1/b/@myBucket/o?uploadType=multipart";

  //token
  String refresh_token = "";
  String client_id = "";
  String client_secret = "";
  String accessToken = "";
  String bucket = "";

  //request
  String start_request_text_base = "";            //リクエストごとにファイル名・親フォルダID・コメントを差し替える
  String start_request_jpeg_base = "";            //リクエストごとにファイル名・親フォルダID・コメントを差し替える
  String end_request = "\r\n--foo_bar_baz--\r\n"; //どのリクエストでも共通

public:
  String postHeader_base_drive = "";   //リクエストごとにサイズを差し替える
  String postHeader_base_storage = ""; //リクエストごとにサイズを差し替える
  String parentFolder = "";

  const char *host = "www.googleapis.com";
  const int httpsPort = 443;

  String getPostHeader(String header, uint32_t len)
  {
    if (accessToken.length() > 0)
    {
      String tmp = header;
      tmp.replace("@full_length", String(len));
      return tmp;
    }
    else
    {
      return "";
    }
  }

  String getStartRequest_Text(String _fileName, String _comment)
  {
    if (accessToken.length() > 0)
    {
      String tmp = start_request_text_base;
      tmp.replace("@fileName", _fileName);
      tmp.replace("@comment", _comment);
      return tmp;
    }
    else
    {
      return "";
    }
  }

  String getStartRequest_Jpeg(String _fileName, String _comment)
  {
    if (accessToken.length() > 0)
    {
      String tmp = start_request_jpeg_base;
      tmp.replace("@fileName", _fileName);
      tmp.replace("@comment", _comment);
      return tmp;
    }
    else
    {
      return "";
    }
  }

  String getEndRequest()
  {
    if (accessToken.length() > 0)
    {
      return end_request;
    }
    else
    {
      return "";
    }
  }

  bool InitAPI()
  {
    Nefry.setStoreTitleStr("Storage Bucket", NEFRY_DATASTORE_BUCKET);
    Nefry.setStoreTitleStr("Refresh Token", NEFRY_DATASTORE_REFRESH_TOKEN);
    Nefry.setStoreTitleStr("Client ID", NEFRY_DATASTORE_CLIENT_ID);
    Nefry.setStoreTitleStr("Client Secret", NEFRY_DATASTORE_CLIENT_SECRET);
    Nefry.setStoreTitleStr("Parent Folder", NEFRY_DATASTORE_PARENT);

    bucket = Nefry.getStoreStr(NEFRY_DATASTORE_BUCKET);
    refresh_token = Nefry.getStoreStr(NEFRY_DATASTORE_REFRESH_TOKEN);
    client_id = Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_ID);
    client_secret = Nefry.getStoreStr(NEFRY_DATASTORE_CLIENT_SECRET);
    parentFolder = Nefry.getStoreStr(NEFRY_DATASTORE_PARENT);
    accessToken = "";

    if (refresh_token == "")
      return false;
    if (client_id == "")
      return false;
    if (client_secret == "")
      return false;

    delay(5000);

    //アクセストークンを取得
    accessToken = GetAccessToken(refresh_token, client_id, client_secret);
    if (accessToken.length() > 0)
    {
      Serial.println(F("Get New AccessToken"));
    }
    else
    {
      Serial.println(F("Fail to Get New AccessToken"));
      return false;
    }

    //リクエストするときのヘッダーなどを設定する
    String tmpHeader = "";
    tmpHeader = tmpHeader + ("POST @URI HTTP/1.1\r\n") +
                ("Host: " + String(host) + ":" + String(httpsPort) + "\r\n") +
                ("Connection: close\r\n") +
                ("Content-Type: multipart/related; boundary=foo_bar_baz\r\n") +
                ("Content-Length: @full_length") +
                ("\r\n") +
                ("Authorization: Bearer " + accessToken + "\r\n") +
                ("\r\n");

    postHeader_base_drive = tmpHeader;
    postHeader_base_drive.replace("@URI", drive_uri);

    postHeader_base_storage = tmpHeader;
    postHeader_base_storage.replace("@URI", storage_uri);
    postHeader_base_storage.replace("@myBucket", bucket);

    start_request_text_base = "";
    start_request_jpeg_base = "";
    String tmp = "";

    tmp = tmp +
          "\r\n--foo_bar_baz\r\n" +
          "Content-Type: application/json; charset=UTF-8\r\n" +
          "\r\n{\r\n" +
          "\t\"name\": \"@fileName\",\r\n" +
          "\t\"parents\": [\"" + parentFolder + "\"],\r\n" +
          "\t\"description\": \"@comment\"\r\n" +
          "}\r\n\r\n" +
          "--foo_bar_baz\r\n";
    start_request_text_base = tmp + "Content-Type: text/plain\r\n\r\n";
    start_request_jpeg_base = tmp + "Content-Type: image/jpeg\r\n\r\n";
  }

  //リフレッシュトークンからアクセストークンを取得
  String GetAccessToken(String refresh_token, String client_id, String client_secret)
  {
    String token = "";
    String postData = "";
    postData += "refresh_token=" + refresh_token;
    postData += "&client_id=" + client_id;
    postData += "&client_secret=" + client_secret;
    postData += "&grant_type=" + String("refresh_token");

    String postHeader = "";
    postHeader += ("POST " + token_uri + " HTTP/1.1\r\n");
    postHeader += ("Host: " + String(host) + ":" + String(httpsPort) + "\r\n");
    postHeader += ("Connection: close\r\n");
    postHeader += ("Content-Type: application/x-www-form-urlencoded\r\n");
    postHeader += ("Content-Length: ");
    postHeader += (postData.length());
    postHeader += ("\r\n\r\n");

    String result = postRequest(host, postHeader, postData);

    //取得したjsonデータからAccessTokenを取得する
    const int BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(1);
    StaticJsonDocument<BUFFER_SIZE> doc;
    char json[result.length() + 1];
    unsigned int size = sizeof(json);
    result.toCharArray(json, sizeof(json));

    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return "";
    }

    const char *tmp = doc["access_token"];
    token = tmp;
    return token;
  }

  //テキストファイルをGoogleDriveにアップロードする
  String postDrive_Text(String _fileName, String _textData, String _comment)
  {
    return postText(_fileName, _textData, _comment, postHeader_base_drive);
  }

  //テキストファイルをGCP Storageにアップロードする
  String postStorage_Text(String _fileName, String _textData)
  {
    return postText(_fileName, _textData, "", postHeader_base_storage);
  }

private:
  //指定のヘッダーを使ってテキストファイルをアップロードする
  String postText(String _fileName, String _textData, String _comment, String header)
  {
    String resMsg = "Something is wrong...";

    Serial.println(F("[Start Post Text]"));

    uint8_t DataSize = _textData.length();
    uint8_t postData[DataSize];
    for (int i = 0; i < DataSize; i++)
    {
      postData[i] = (uint8_t)_textData[i];
    }

    String start_request = getStartRequest_Text(_fileName, _comment);

    uint16_t full_length;
    full_length = start_request.length() + DataSize + end_request.length();
    String postHeader = getPostHeader(header, full_length);

    String result = "";

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("Connecting to: ");
    Serial.println(host);

    if (!client.connect(host, httpsPort))
    {
      resMsg = F("connection failed");
      Serial.println(resMsg);
      return resMsg;
    }

    Serial.println("[Header]");
    Serial.println(postHeader);
    Serial.println("[start_request]");
    Serial.println(start_request);
    Serial.println("[end_request]");
    Serial.println(end_request);
    client.print(postHeader + start_request);

    client.write(&postData[0], DataSize);

    client.println(end_request);

    Serial.println("Receiving response");
    if (client.connected())
    {
      if (client.find("HTTP/1.1 "))
      {
        String status_code = client.readStringUntil('\r');
        Serial.print("Status code: ");
        Serial.println(status_code);
        if (status_code != "200 OK")
        {
          resMsg = F("[ERR] Status code:");
          resMsg += status_code;
          Serial.println(resMsg);
          return resMsg;
        }
      }

      if (client.find("\r\n\r\n"))
      {
        result = client.readStringUntil('\r');
        resMsg = F("Success!!!");
        Serial.println(resMsg);
        Serial.println(result);
      }
      else
      {
        resMsg = F("[WARNING] Response Data is Nothing");
        Serial.println(resMsg);
      }
    }

    Serial.println("closing connection");
    return resMsg;
  }

  //サーバーにデータをポストする
  String postRequest(const char *server, String header, String data)
  {

    String result = "";

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("Connecting to: ");
    Serial.println(server);

    if (!client.connect(server, httpsPort))
    {
      Serial.println("connection failed");
      return result;
    }
    Serial.println("certificate matches");

    Serial.print("post: ");
    Serial.println(header + data);
    client.print(header + data);

    Serial.println("Receiving response");
    if (client.connected())
    {
      if (client.find("HTTP/1.1 "))
      {
        String status_code = client.readStringUntil('\r');
        Serial.print("Status code: ");
        Serial.println(status_code);
        if (status_code != "200 OK")
        {
          Serial.println("There was an error");
        }
      }
      if (client.find("\r\n\r\n"))
      {
        Serial.println(F("[Read Data]"));
      }
      String line = client.readStringUntil('\r');
      Serial.println(line);
      result += line;
    }

    Serial.println("closing connection");
    return result;
  }
};
#endif
