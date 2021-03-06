# BBED使用

```
BBED可以将二进制块打印出来，也可以自己写demo将二进制读出来。
```
## 安装
&emsp;&emsp;11G中缺少bbed包，需要上传sbbdpt.o, ssbbded.o, bbedus.msb ,将这三个文件放在以下目录中：

```
$ ORACLE_HOME/rdbms/lib/ssbbded.o
$ ORACLE_HOME/rdbms/lib/sbbdpt.o
$ ORACLE_HOME/rdbms/mesg/bbedus.msb
```
&emsp;&emsp;后执行如下命令：

```
$ cd $ORACLE_HOME/rdbms/lib
$ make -f $ORACLE_HOME/rdbms/lib/ins_rdbms.mk BBED=$ORACLE_HOME/bin/bbed $ORACLE_HOME/bin/bbed
```

## 使用
1.新建dbfiles.txt(文件名随意), 填入文件信息：

```
1       /home/oracle/app/oradata/orcl/redo03.log        52429312
[文件编号]	[文件路径]	[文件大小]
```
2.新建参数文件para.txt(文件名随意)，内容如下：

```
blocksize=512
listfile=dbfiles.txt
mode=edit

```
3.我一般将dbfiles.txt和para.txt放在$ORACLE_BASE/bbed(路径随意)下, 启动bbed:

```
$ cd $ORACLE_BASE/bbed
$ bbed parfile=para.txt --> [密码是:blockedit]
```

4.bbed一些基本操作

```
BBED> info
 File#  Name                                                        Size(blks)
 -----  ----                                                        ----------
     1  /home/oracle/app/oradata/orcl/system01.dbf                       89600
     2  /home/oracle/app/oradata/orcl/sysaux01.dbf                       76800
```

```
BBED> show all
	FILE#          	1
	BLOCK#         	1
	OFFSET         	0
	DBA            	0x00400001 (4194305 1,1)
	FILENAME       	/home/oracle/app/oradata/orcl/system01.dbf
	BIFILE         	bifile.bbd
	LISTFILE       	dbfiles.txt
	BLOCKSIZE      	8192
	MODE           	Edit
	EDIT           	Unrecoverable
	IBASE          	Dec
	OBASE          	Dec
	WIDTH          	80
	COUNT          	512
	LOGFILE        	log.bbd
	SPOOL          	No
```

```
如果要查看5号文件的2号块，需要通过set命令来设置文件号和块号

BBED> set file 5
	FILE#          	5

BBED> set block 2
	BLOCK#         	2

BBED> dump
 File: /home/oracle/app/oradata/orcl/redo01.log (5)
 Block: 2                Offsets:    0 to  511           Dba:0x01400002
------------------------------------------------------------------------
 01220000 02000000 34000000 108000c8 a8010000 05180000 732b0b00 01000206 
 c50f3b25 545402c1 00000100 89010000 89010000 0875030f 732b0b00 0000ac9c 
 007d0002 06c50f3b 7b2b0b00 00000000 1689f337 05021700 0300ffff b000c000 
 702b0b00 00000000 0200ffff 04002000 01000000 8f020000 7203c000 91000100 
 0a048800 ff7f0000 00000000 00000000 05011800 0300ffff 7203c000 702b0b00 
 00008672 0101ffff 0c001400 4c002000 0c000100 88000000 0a000000 04000100 
 8f020000 910001ad dd030100 dd030100 01000000 00000000 0a150100 080c0100 
 00000000 b700c000 90000300 97250b00 0000bdad 99250b00 0000bdad ff7f0000 
 ffffffff ffff0000 7303c000 00000000 00000000 040d8672 00000000 04000300 
 90020000 7a01c000 8d000100 00c00000 a4250b00 07198672 84b08000 1aaf8000 
 01ac9c00 0a060100 02000100 84b08000 702b0b00 000067bf 0100dd03 06001800 
 02000000 010d94bf 097f0000 04000100 8f020000 7203c000 91000100 01051506 
 05140000 00000000 00000000 00000000 00000000 00060000 10000400 00000200 
 04000400 00000000 2d00c400 04000080 0001200b 00000000 a4200000 01020000 
 732b0b00 010003c2 15060778 75030f0f 05021700 0300ffff b000c000 732b0b00 
 0000c000 0100ffff 04002000 01008abf 00000000 7303c000 91000100 0a00d41f 

 <32 bytes per line>

```
## 自己写的打印程序
&emsp;&emsp;demo上传到./demo/tools/dump.cc,编译后使用时需要传入文件名，起始块号，dump块数。例：

```
$ clang++ dump.cc -o dump
$ ./dump /Users/zhoubihui/redo/redo01.log 2 4
表示dump 2,3,4,5块。
```


# 转储日志文件工具-dump
## 使用
&emsp;&emsp;个人使用习惯如下：  
1.执行DML操作之前先查询当前日志文件的序列号和块号

```
SQL> select cpodr_seq,cpodr_bno from x$kcccp where rownum=1;

 CPODR_SEQ  CPODR_BNO
---------- ----------
	 7	  249

--------------------执行DML--------------------------------------

SQL> select cpodr_seq,cpodr_bno from x$kcccp where rownum=1;             

 CPODR_SEQ  CPODR_BNO
---------- ----------
	 7	  743

SQL> oradebug setmypid;
Statement processed.

SQL> alter system dump logfile '/home/oracle/app/oradata/orcl/redo01.log' rba min 7 249 rba max 7 743;

System altered.

SQL> oradebug tracefile_name;
/home/oracle/app/diag/rdbms/orcl/orcl/trace/orcl_ora_3434.trc
```

```
1.需要dump的文件，可以联合v$log和v$logfile查询文件路径。
2.dump的方式有很多种，这种是我最常用的同时也是我觉得比较方便的。
3.每次dump需要退出连接，因为生成的文件名是根据会话ID生成的，不重新登录的话，所有dump的数据都在同一个文件里，不够直观。
4.如果只执行一个insert操作，但是redo写了几百个块，例如上述例子中，这个时候说明oracle自身在对数据库做一些操作，如果新手在这几百个块中找一条insert记录的话，是比较难找的。所以可以通过多执行几次上述操作，只要写的redo块数比较少，dump出来的数据也是比较容易找到自己想要的那条记录。
```

## 例子

```
REDO RECORD - Thread:1 RBA: 0x000007.000000f9.0010 LEN: 0x0224 VLD: 0x0d
SCN: 0x0000.000fbebd SUBSCN:  1 04/04/2019 18:22:15
CHANGE #1 TYP:2 CLS:1 AFN:1 DBA:0x004007d9 OBJ:287 SCN:0x0000.000fbea6 SEQ:2 OP:11.5 ENC:0 RBL:0
KTB Redo
op: 0x11  ver: 0x01
compat bit: 4 (post-11) padding: 1
op: F  xid:  0x0009.001.00000330    uba: 0x00c029d9.0086.24
Block cleanout record, scn:  0x0000.000fbebc ver: 0x01 opt: 0x02, entries follow...
  itli: 2  flg: 2  scn: 0x0000.000fbea6
KDO Op code: URP row dependencies Disabled
  xtype: XA flags: 0x00000000  bdba: 0x004007d9  hdba: 0x004007d8
itli: 1  ispac: 0  maxfr: 4863
tabn: 0 slot: 2(0x2) flag: 0x2c lock: 1 ckix: 11
ncol: 19 nnew: 1 size: 7
col  5: [ 7]  78 77 04 04 13 17 10
CHANGE #2 TYP:0 CLS:33 AFN:3 DBA:0x00c00100 OBJ:4294967295 SCN:0x0000.000fbe8b SEQ:2 OP:5.2 ENC:0 RBL:0
ktudh redo: slt: 0x0001 sqn: 0x00000330 flg: 0x0012 siz: 156 fbi: 0
            uba: 0x00c029d9.0086.24    pxid:  0x0000.000.00000000
CHANGE #3 TYP:0 CLS:33 AFN:3 DBA:0x00c00100 OBJ:4294967295 SCN:0x0000.000fbebd SEQ:1 OP:5.4 ENC:0 RBL:0
ktucm redo: slt: 0x0001 sqn: 0x00000330 srt: 0 sta: 9 flg: 0x2 ktucf redo: uba: 0x00c029d9.0086.24 ext: 2 spc: 3270 fbi: 0
CHANGE #4 TYP:0 CLS:34 AFN:3 DBA:0x00c029d9 OBJ:4294967295 SCN:0x0000.000fbe8b SEQ:3 OP:5.1 ENC:0 RBL:0
ktudb redo: siz: 156 spc: 3428 flg: 0x0012 seq: 0x0086 rec: 0x24
            xid:  0x0009.001.00000330
ktubl redo: slt: 1 rci: 0 opc: 11.1 [objn: 287 objd: 287 tsn: 0]
Undo type:  Regular undo        Begin trans    Last buffer split:  No
Temp Object:  No
Tablespace Undo:  No
             0x00000000  prev ctl uba: 0x00c029d9.0086.21
prev ctl max cmt scn:  0x0000.000f6a42  prev tx cmt scn:  0x0000.000f6a5a
txn start scn:  0x0000.00000000  logon user: 0  prev brb: 12593621  prev bcl: 0 BuExt idx: 0 flg2: 0
KDO undo record:
KTB Redo
op: 0x04  ver: 0x01
compat bit: 4 (post-11) padding: 1
op: L  itl: xid:  0x0001.000.0000026a uba: 0x00c02332.0075.04
                      flg: C---    lkc:  0     scn: 0x0000.000fbe9a
KDO Op code: URP row dependencies Disabled
  xtype: XA flags: 0x00000000  bdba: 0x004007d9  hdba: 0x004007d8
itli: 1  ispac: 0  maxfr: 4863
tabn: 0 slot: 2(0x2) flag: 0x2c lock: 0 ckix: 11
ncol: 19 nnew: 1 size: -7
col  5: *NULL*
```
