  ゲームデータインストール用スクリプト instgame の使い方について

                                    スクリプト作成   奈良木 さん
                                    ドキュメント作成 近間@NAIST
0. 目次

  1. ファイルの説明
  2. 使い方
  3. その他の情報
  4. 著作権など
  5. 連絡先


1. ファイルの説明

  instgame       スクリプト本体
  456atlach.inf  ATLACH-NACHA 用設定ファイル (アリスの館456収録)
  456ningen.inf  人間狩り 用設定ファイル (アリスの館456収録 
  456zero.inf    零式 用設定ファイル (アリスの館456収録)
  ambi.inf       AmbivalenZ for WIN95 用設定ファイル
  atlach_new.inf ATLACH-NACHA (廉価版) 用設定ファイル
  darcrows.inf   DARCROWS 用設定ファイル
  diabo.inf      DiaboLiQue 用設定ファイル
  diabo_new.inf  DiaboLiQue (廉価版) 用設定ファイル
  hushaby.inf    HushaBy Baby 用設定ファイル
  katsumi.inf    いけないかつみ先生 用設定ファイル
  kichiku.inf    鬼畜王ランス 用設定ファイル
  mamatoto.inf   ママトト 用設定ファイル
  mamori.inf     守り神様 用設定ファイル
  miko.inf       戦巫女 用設定ファイル
  mugen.inf      無幻泡影 for WIN95 用設定ファイル
  oudou.inf      王道勇者 用設定ファイル
  panyo.inf      かえるにょぱにょ〜ん 用設定ファイル
  panyo_new.inf  かえるにょぱにょ〜ん(廉価版) 用設定ファイル
  pascha.inf     ぱすてるチャイム 用設定ファイル
  persiom.inf    PERSIOM 用設定ファイル
  progood.inf    ぷろすちゅーでんとGOOD 用設定ファイル
  rance4.inf     ランス４ forWIN 用設定ファイル
  seeinao.inf    SeeIn青 用設定ファイル
  toushin2.inf   闘神都市II forWIN 用設定ファイル
  dalk.inf       DALK 用設定ファイル (アリスの館456収録)
  drstop.inf     Dr.Stop! 用設定ファイル (アリスの館456収録)
  rance1.inf     Rance1 用設定ファイル (アリスの館456収録)
  rance2.inf     Rance2 用設定ファイル (アリスの館456収録)
  rance3.inf     Rance3 用設定ファイル (アリスの館456収録)
  toushin.inf    闘神都市 用設定ファイル (アリスの館456収録)
  zero_new.inf   零式 (廉価版) 用設定ファイル
  20cKaeru.inf   20世紀アリス かえるにょ国にょアリス 用設定ファイル
  20cDPS.inf     20世紀アリス これDPS？ 用設定ファイル
  onlyyou_r.inf  Only You リクルス 用設定ファイル
  daiakuji.inf   大悪司 用設定ファイル
  tsuma.inf      妻みぐい 用設定ファイル

2. 使い方

   ゲームのCD-ROMをドライブに入れてマウントし、
 %  ./instgame kichiku.inf
 のように実行します。

   デフォルトではCDROMは/cdromにマウントされているものとしています。
 また、データのインストール先は ~/game/kichiku/ の下になります。

   デフォルトでは設定用ファイルのファイル名が~/game/の下のディレクトリ名に
 なります。

  これらのデフォルトの条件を変えるには以下のオプションを使います。

  -c : CD-ROMのマウントされている位置を指定します(default: -c /cdrom)
  -d : ゲームデータのインストール先を指定します  (default: -d ~/game/target>)

   データが正常にコピーされると、~/game/の下にkichiku.grと言うファイルが作成
 されます。これはゲームデータの場所をプログラム(xsystem35)に知らせるためのも
 ので、
 % xsystem35 -gamefile ~/game/kichiku.gr
 のように指定するのに使います。(「ゲームリソースファイル」とでも呼んで下さい)

   デフォルトでは、*.grというファイルも設定用ファイルのファイル名と同じ名前に
 なりますが、以下のオプションで変更可能です。

  -g : ゲームリソースファイルのファイル名を指定します
       (default: -g ~/game/<target>.inf)


3. その他の情報

   その他のオプションに、指定のファイルはCD-ROMからコピーせずにリンクを張る、
 という指定が出来ます。

 % ./instgame -k Graphics kichiku.inf

 とすると、kichiku_ga.ald と kichiku_gb.ald のファイルはコピーされずに、
 CD-ROM上のデータへのシンボリックリンクになります。ただし、そうするとゲーム中
 はCD-ROMをマウントしておかなければならないので、CD-ROMをマウントしたまま
 CD-DAの演奏可能な一部のシステムでのみ可能です。


   このオプションはゲームデータのインストール時の動作を規定するものなので、
 自分でCD-ROMからファイルをコピーして、ゲームリソースファイルをエディタで
 編集すれば、全く自由にデータの配置を行なう事が出来ます。もちろん、ゲーム
 リソースファイルのファイル名や置き場所も自由にして出来ます。
 ゲームリソースファイルの書式については doc/GRFMT.TXT を御覧下さい。


4. 著作権など

   instgameは 奈良木 さんがつくられたものですので、著作権は奈良木さんに
 属します。奈良木さんの意向により、GPLに基づいて再配布可能です。

  ドキュメントの不備は私(近間)までどうぞ。


5. 連絡先

   奈良木さん <naraki@debian.or.jp>
   近間@NAIST <masaki-c@is.aist-nara.ac.jp>
