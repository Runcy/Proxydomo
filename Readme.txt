/* ===================================
　　　　　　　Proxydomo　　　　　　　　　　　　
 ==================================== */
 
■はじめに
このソフトはローカルで動くプロクシフィルタリングソフトです
作成にあたりオープンソースのProximodoを使って作られています。

■使い方
起動して　127.0.0.1:6060(プロクシポートに表示されてる数値)をプロクシとして指定すると
プロクシフィルタとして機能します。
詳しい使い方はProxomitronなどを参考にしてください。

$LSTの指定方法は listsフォルダ以下にあるテキストから拡張子を消したものです
※例: lists\Kill.txt -> $LST(Kill)

■既知のバグ
・一部実装していないコマンドがあります($ADDLSTBOXなど)

■免責
作者(原著者＆改変者)は、このソフトによって生じた如何なる損害にも、
修正や更新も、責任を負わないこととします。
使用にあたっては、自己責任でお願いします。
 
何かあれば下記のURLにあるメールフォームにお願いします。
http://www31.atwiki.jp/lafe/pages/33.html
 
■著作権表示
Copyright (C) 2004 Antony BOUCHER
Copyright (C) 2013 amate
 
 画像の一部に「VS2010ImageLibrary」の一部を使用しています。
 
■ビルドについて
ビルドには boost(1.54~)と zlib と WTL と ICU と wolfSSL が必要なのでそれぞれ用意してください。

◆boost
http://www.boost.org/

◆zlib
http://zlib.net/

◆ICU
http://site.icu-project.org/

◆WTL
http://sourceforge.net/projects/wtl/

◆wolfSSL
http://www.wolfssl.com/yaSSL/Home.html

□コンパイル済みdll
zlibのコンパイル済みdllを下記のURLで公開しています
http://1drv.ms/1vqvcaG


zlibのヘッダの場所
$(SolutionDir)zlib\x86\include
$(SolutionDir)zlib\x64\include
zlibのライブラリの場所
$(SolutionDir)zlib\x86\lib
$(SolutionDir)zlib\x64\lib
を以下の場所にすればとくに設定はいらないはずです
これ以外の場所にzlibを置いているなら適当にzlibbuffer.h/cppを修正してください

ICU は
$(SolutionDir)icu\Win32 or Win64 フォルダに include と lib があればコンパイル通るようになっています

boost::shared_mutexを使用するのでboost::threadのライブラリが必要になります
 Boostライブラリのビルド方法
 https://sites.google.com/site/boostjp/howtobuild
コマンドライン
// x86
b2.exe install -j 2 --prefix=lib toolset=msvc-12.0 runtime-link=static --with-thread --with-date_time
// x64
b2.exe install -j 2 --prefix=lib64 toolset=msvc-12.0 runtime-link=static address-model=64 --with-thread --with-date_time


v1.55の以下の修正はwolfSSL側のソースを修正する必要があります
・サーバーから送られてくるルートCA証明書がおかしいとき"ASN no signer error to confirm failure"の警告が出るのを修正
internal.cの 3990行目あたりを

// before
WOLFSSL_MSG("Failed to verify CA from chain");

// after
WOLFSSL_MSG("Failed to verify CA from chain");

if (count == totalCerts && count > 2) {
	if (ret == ASN_NO_SIGNER_E || ret == ASN_SIG_CONFIRM_E) {
		ret = 0;	// 最上位のみ見逃す
	}
}
// ==============================================

