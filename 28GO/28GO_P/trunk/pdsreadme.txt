28GO_G 0030                                                       by  hideyosi

・これは何か？

　これは川合秀実氏が発表していたgo(最終版は0023）を元にhideyosiが派生させたもの
　です。
　
　
・GO(0023)とは？
　川合氏がgcc-3.2を改造して作ったCコンパイラ・自作した独自アセンブラ・その他
　開発に必要なツールのセット名です。
　（主にOSASKの開発用に特化・チューンナップされたgccと考えてください。
　　もちろん普通のアプリケーションも開発は可能です。）
　
　
・28GO_Gや28GO_K等複数のパッケージがあるが？
　コンパイラ等の開発ツールはそれひとつポンとあれば良いわけではなく、様々な
　ツールやライブラリがひとつになって働くものです。
　しかし、goの場合はgccを改造したものと川合氏がゼロから興したもの。PDSと宣言
　されている物等が混在しています。
　ライセンスに従うこと。区別がつくようにとパッケージを分けています。
　各々は以下のようになっています。
　
　　　・28GO_K　　　　KL-01ライセンスのもの
　　　・28GO_G　　　　GPLランセンスのもの(一部例外付きGPLの物を含んでいます）
　　　・28GO_P　　　　作者がPDS宣言しているもの
　　　・28GO_U　　　　GNUのツール郡です。（単純に配布元からコピーしたもの）

　※　このソースパッケージは上記の28GO_P内のもののソースです。



・ライセンスについて
　このパッケージ内のものは全てGPLが適用されます。
　
　・例外付きGPL
　　w32clibc/libmingw_o/cygwin.asmは例外付きGPLです。詳しくはソース内に書かれて
　　います。（英語）

　GPLのライセンス文はCopyingにあります。
　
　GPL・例外付きGPL部分の著作権については、それぞれの元著作者に帰します。
　
　
・ご注意
　28GOはgo0023(2004/5/6リリース）の代替として配布していますが多くのソフトは既に
　新しいバージョン・または別物のソフトに生まれ変わっていたりします。
　（2009/12/13現在）
　最新のものについてはhttp://osask.net/をご覧ください。


・謝辞

　gccの開発者のみなさまに心からの謝辞をささげます。もしこのgoの一部がgccの開発
　者の方々に認めていただけたら、こんなに嬉しいことはありません。

　Gakuさんのstringライブラリを使わせていただきました。Gakuさん、ありがとうござ
　います。

　また開発を直接助けてくれた、くーみんさん、henohenoさん、
　そしてOSASKコミュニティーのみなさま、ありがとうございました。

　なお、川合の2002.10.03〜2003.02.28の期間のおける開発成果は、
　　　　　特別認可法人 情報処理振興事業協会 (IPA) 
　の「未踏ソフトウェア創造事業未踏ユース」での委託業務によるものです。この場を
　借りて支援をいただけたことにたいしてお礼申し上げます。


・免責
　このアーカイブはコミュニティメンバーであるhideyosiがオープンソース
　(KL-01・GPL)であるgo_0023を元に派生させたものです。
　
　「公開していること」における責任はhideyosiにありますので、原作者である
　川合氏に対して当アーカイブに関するお問い合わせ等はなさらないでください。
　
　hideyosiは多くのフリーソフト・オープンソースソフトと同じく、これを使用
　することによって起こるいかなる不具合・不利益に対しても責任を負いかねます。
　また、保守・存続・質疑応答・バグ等についても責任・義務を負いません。



・お問い合わせは？

　http://osask.net/（OSASKコミュニティサイト）の掲示板等でお願いいたします。

　hideyosi自身はとてもスキルが低い者です。技術的な部分に関してのお問い合わせを
　頂いてもほとんどお答えすることは出来ないでしょう。（申し訳ありません）
　
　しかしコミュニティサイト、http://osask.net/内には過去に川合氏が書いた文章が
　豊富にストックされ、またhideyosiよりも優れた方々が沢山出入りしています。
　掲示板やWiki等を設置しておりますのでこちらにてお問い合わせ等をして頂ければ
　なにかしらの返答が期待できるかもしれませんので是非ご活用ください。


　　　　　　　　　　　　　　2010.1.8  hideyosi  http://osask.net/w/502.html
