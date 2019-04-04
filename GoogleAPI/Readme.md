# NefryBTでGoogleAPIを使う  
## 下準備  
### DataStoreに必要な情報を登録する  
<table>
    <tr>
        <td>番号</td>
        <td>値</td>
    </tr>
    <tr>
        <td>4</td>
        <td>バケット名</td>
    </tr>
    <tr>
        <td>5</td>
        <td>リフレッシュトークン</td>
    </tr>
    <tr>
        <td>6</td>
        <td>クライアントID</td>
    </tr>
    <tr>
        <td>7</td>
        <td>クライアントシークレット</td>
    </tr>
    <tr>
        <td>8</td>
        <td>GoogleDriveのとき親フォルダID・Storageのときフォルダ名</td>
    </tr>
</table>
  
## 関数
### bool InitAPI()  
* 初期設定  
* DataStoreの設定、アクセストークンの取得、リクエストするときのヘッダーの設定
* エラーの場合Falseを返す。
  
  
### String GetAccessToken(String refresh_token, String client_id, String client_secret)  
* リフレッシュトークンからアクセストークンを取得  
* 引数は　リフレッシュトークン、クライアントID、クライアントシークレット
* アクセストークンが返ってくる。  

### void postDrive_Text(String _fileName, String _textData,  String _comment)  
* テキストファイルをGoogleDriveにアップロードする
* 引数は　ファイル名、テキストデータ、コメント(ファイルの簡単な説明)
  
### void postStorage_Text(String _fileName, String _textData)  
* テキストファイルをGCPのStorageにアップロードする
* 引数は　ファイル名、テキストデータ

### リクエスト時に使う設定を取得する関数  
#### String getPostHeader(uint32_t len) : ヘッダー　POSTするデータサイズを指定  
#### String getStartRequest_Text(String _fileName, String _comment) : テキストをPOSTするときのリクエスト(開始部分)  
#### String getStartRequest_Jpeg(String _fileName, String _comment) : JpegをPOSTするときのリクエスト(開始部分)  
#### String getEndRequest() : POSTするときのリクエスト(終了部分)  

## ポイント  
### アクセストークンは1時間(3600秒)で無効になる  
なので(今のところ)毎回リフレッシュトークンから再生成する  
### GoogleDriveにポストするときの形式  
```
POST /upload/drive/v3/files?uploadType=multipart HTTP/1.1
Host: www.googleapis.com:443
Connection: close
Content-Type: multipart/related; boundary=foo_bar_baz
Content-Length: [データの長さ]
Authorization: Bearer [アクセストークン]

--foo_bar_baz
Content-Type: application/json; charset=UTF-8

{
	"name": "[ファイル名]",
	"parents": ["[親フォルダID]"],
	"description": "[コメント]"
}

--foo_bar_baz
Content-Type: text/plain

[ファイルの中身]

--foo_bar_baz--

```  
* POST～ 部分を"getPostHeader"で生成している
* --foo_bar_baz～ 部分を"getStartRequest_Text"で生成している
* --foo_bar_baz-- を"getEndRequest"で生成している
* [ファイルの中身]は実際のデータ("hogehoge"など)を送る
### 上記を踏まえた処理の順番  
1. ヘッダーを送信する(client.print)
1. リクエストの開始部分を送信する(client.print)
1. **データの中身を送信する(client.write)**
1. リクエストの終了部分を送信する(client.print)  
  
※データの中身を送信するところが厄介。  膨大なデータ量(Jpegデータだと10万バイト以上ある)の場合、変数に保存できないので小分けにして取得→送信を繰り返す。  

### GCP Storageにポストするときの形式
POSTするURLが違うだけ。
```
POST /upload/storage/v1/b/myBucket/o?uploadType=multipart HTTP/1.1
Host: www.googleapis.com:443
Connection: close
Authorization: Bearer [YOUR_AUTH_TOKEN]
Content-Type: multipart/related; boundary=foo_bar_baz
Content-Length: [NUMBER_OF_BYTES_IN_ENTIRE_REQUEST_BODY]


--foo_bar_baz
Content-Type: application/json; charset=UTF-8


{
  "name": "myObject"
}


--foo_bar_baz
Content-Type: image/jpeg


[JPEG_DATA]
--foo_bar_baz--
```
