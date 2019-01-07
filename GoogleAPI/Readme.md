# NefryBTでGoogleAPIを使う  
## 下準備  
### DataStoreに必要な情報を登録する  
<table>
    <tr>
        <td>番号</td>
        <td>値</td>
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
        <td>親フォルダID</td>
    </tr>
</table>
  
## 関数
### bool InitAPI()  
* 初期設定  
* DataStoreの設定、アクセストークンの取得、リクエストするときのヘッダーの設定
  
  
### String GetAccessToken(String refresh_token, String client_id, String client_secret)  
* リフレッシュトークンからアクセストークンを取得  
* 引数は　リフレッシュトークン、クライアントID、クライアントシークレット
* アクセストークンが返ってくる。  

### void postDrive_Text(String _fileName, String _textData,  String _comment)  
* テキストファイルをGoogleDriveにアップロードする
* 引数は　ファイル名、テキストデータ、コメント(ファイルの簡単な説明)
  
### リクエスト時に使う設定を取得する関数  
#### String getStartRequest_Text(String _fileName, String _comment) : テキストをPOSTするときのリクエスト(開始部分)  
#### String getStartRequest_Jpeg(String _fileName, String _comment) : JpegをPOSTするときのリクエスト(開始部分)  
#### String getPostHeader(uint32_t len) : ヘッダー　POSTするデータサイズを指定  
#### String getEndRequest() : POSTするときのリクエスト(終了部分)  

