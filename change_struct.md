# 常见change分析
## opcode=11.2,insert操作

```
SQL> insert into t1 values (2, 'wsdecx');


CHANGE #1 TYP:2 CLS:1 AFN:1 DBA:0x00414f01 OBJ:73439 SCN:0x0000.000f5aba SEQ:5 OP:11.2 ENC:0 RBL:0

KTB Redo 
op: 0x01  ver: 0x01  
compat bit: 4 (post-11) padding: 1
op: F  xid:  0x0003.01b.00000330    uba: 0x00c02886.00ac.36

KDO Op code: IRP row dependencies Disabled
  xtype: XA flags: 0x00000000  bdba: 0x00414f01  hdba: 0x00414f00
itli: 2  ispac: 0  maxfr: 4863
tabn: 0 slot: 1(0x1) size/delt: 13
fb: --H-FL-- lb: 0x2  cc: 2
null: --

col  0: [ 2]  c1 03
col  1: [ 6]  77 73 64 65 63 78

insert操作产生的change可以分成3部分,KTB-KDO-实际数据,其中KTB记录了事务ID和UBA等信息,KDO记录了表号,行号slot,数据块地址bdba,数据段地址hdba等,第三部分的col 0,col 1即本次insert操作的数据。
其中KTB的结构根据op参数的值不同,有不同的结构。

KTB:请看KTB_struct.md。

KDO结构:
typedef struct KDO{
	uint32_t bdba;
	uint32_t hdba;
	uint16_t maxfr;
	uint16_t unknown0;
	uint8_t itli;   
	uint24_t unknown1;
	uint8_t fb;   //标志
	uint8_t lb;
	uint8_t cc;
	uint8_t unknown2;
	uint32_t unknown3[5];
	uint16_t size/delt;
	uint16_t slot;  //行号
	uint8_t tabn;  //表编号,簇表中使用
	uint24_t unknown4;
	uint32_t unknown5;
}KDO;

第三部分,是本次insert实际影响了的数据,从表的第一个字段开始。其中c1 03是2(类型转换后续会讲解),77 73 64 65 63 78是ASCII码存储，转换后是'wsdecx'。
```

## opcode=11.3,delete操作
```
SQL> delete from t1 where id=2;

1 row deleted.

CHANGE #1 TYP:2 CLS:1 AFN:1 DBA:0x00414f01 OBJ:73439 SCN:0x0000.000f6468 SEQ:2 OP:11.3 ENC:0 RBL:0

KTB Redo 
op: 0x01  ver: 0x01  
compat bit: 4 (post-11) padding: 1
op: F  xid:  0x0006.001.0000033e    uba: 0x00c001ad.00d1.33

KDO Op code: DRP row dependencies Disabled
  xtype: XA flags: 0x00000000  bdba: 0x00414f01  hdba: 0x00414f00
itli: 2  ispac: 0  maxfr: 4863
tabn: 0 slot: 1(0x1)

delete操作产生的change分成2部分,KTB-KDO,因为根据rowid(后续会讲)可以唯一确定一行，所以不需要再记录其他信息了。

KTB:请看KTB_struct.md。

KDO:
typedef struct KDO{
	uint32_t bdba;
	uint32_t hdba;
	uint16_t maxfr;
	uint16_t unknown0;
	uint8_t itli;
	uint24_t unknown1;
	uint16_t slot;
	uint16_t unknown2
}KDO;
```

## opcode=11.5,update操作
```
SQL> update t2 set id=34,name='wertyu' where id1=22;

CHANGE #1 TYP:2 CLS:1 AFN:1 DBA:0x00414f09 OBJ:73444 SCN:0x0000.000f6832 SEQ:2 OP:11.5 ENC:0 RBL:0

KTB Redo 
op: 0x11  ver: 0x01  
compat bit: 4 (post-11) padding: 1
op: F  xid:  0x0006.00f.00000347    uba: 0x00c001da.00d1.31
Block cleanout record, scn:  0x0000.000fd835 ver: 0x01 opt: 0x02, entries follow...
  itli: 2  flg: 2  scn: 0x0000.000f6832

KDO Op code: URP row dependencies Disabled
  xtype: XA flags: 0x00000000  bdba: 0x00414f09  hdba: 0x00414f08
itli: 1  ispac: 0  maxfr: 4863
tabn: 0 slot: 0(0x0) flag: 0x2c lock: 1 ckix: 0
ncol: 3 nnew: 2 size: -1

col  0: [ 2]  c1 23
col  2: [ 6]  77 65 72 74 79 75

对于一个update操作,分成四个部分:KTB、KDO、update更新的字段编号、update更新的字段值。

KTB:请看KTB_struct.md。

KDO:
typedef struct KDO{
	uint32_t bdba;
	uint32_t hdba;
	uint16_t maxfr;
	uint16_t unknown0;
	uint8_t itli;  //位数不确定
	uint24_t unknown1;
	uint8_t flag;
	uint8_t lock;
	uint8_t ckix;
	uint8_t tabn;
	uint16_t slot;
	uint8_t ncol;
	uint8_t nnew;
	uint...[不知道是什么]
}KDO;

第三部分:
	update操作的字段编号，总长度由向量表中计算，其中2个字节表示一个字段编号。
	
第四部分:
	update操作的字段内容，总长度由向量表中决定。
	假设本次update更新了三个字段的值，则在向量表中字段编号后还有3个2字节，表示三个字段的值的长度。(不理解的可以跳到本文末尾看计算例子)。
```

## opcode=5.2, 操作回滚段头

```
CHANGE #2 TYP:0 CLS:21 AFN:3 DBA:0x00c000a0 OBJ:4294967295 SCN:0x0000.000fc606 SEQ:2 OP:5.2 ENC:0 RBL:0

ktudh redo: slt: 0x0001 sqn: 0x00000338 flg: 0x000a siz: 136 fbi: 0
            uba: 0x00c028b0.00ac.01    pxid:  0x0000.000.00000000
            
对于一个5.2操作，表示一个事务的开始，分成一个部分，记录本次事务分配的第一个undo块的地址，以及事务id(即xid)。

ktudh:
typedef struct KTUDH{
	uint16_t slt;   //xid1
	uint16_t unknown0;
	uint32_t sqn;   //xid2
	uint32_t uba0;
	uint16_t uba1;
	uint8_t uba2;
	uint8_t unknown1;
	uint16_t flg;
	uint16_t siz;
	uint16_t pxid0;
	uint16_t pxid1;
	uint32_t pxid2;
}KTUDH;
```

## opcode=5.1, 记录前镜像数据

```
CHANGE #4 TYP:1 CLS:22 AFN:3 DBA:0x00c028b0 OBJ:4294967295 SCN:0x0000.000fc634 SEQ:1 OP:5.1 ENC:0 RBL:0

ktudb redo: siz: 136 spc: 0 flg: 0x000a seq: 0x00ac rec: 0x01
            xid:  0x0003.001.00000338  

ktubl redo: slt: 1 rci: 0 opc: 11.1 [objn: 73439 objd: 73439 tsn: 0]
Undo type:  Regular undo        Begin trans    Last buffer split:  No 
Temp Object:  No 
Tablespace Undo:  No 
             0x00000000  prev ctl uba: 0x00c028af.00ac.31 
prev ctl max cmt scn:  0x0000.000fc23e  prev tx cmt scn:  0x0000.000fc24d 
txn start scn:  0x0000.000fc500  logon user: 0  prev brb: 12593322  prev bcl: 0 BuExt idx: 0 flg2: 0

KDO undo record:
KTB Redo 
op: 0x04  ver: 0x01  
compat bit: 4 (post-11) padding: 1
op: L  itl: xid:  0x0001.006.00000267 uba: 0x00c00133.0074.2f
                      flg: C---    lkc:  0     scn: 0x0000.000f65c2

KDO Op code: DRP row dependencies Disabled
  xtype: XA flags: 0x00000000  bdba: 0x00414f01  hdba: 0x00414f00
itli: 1  ispac: 0  maxfr: 4863
tabn: 0 slot: 1(0x1)

对于一个5.1操作，随着其他操作而变化，举例：执行一个11.2时，产生的5.1是分成4个部分的(ktudb、ktubl/ktubu、KTB、KDO), 执行一个11.3时，产生的5.1是分成5个部分的(ktudb、ktubl/ktubu、KTB、KDO、实际数据)，具体情况具体分析。

ktudb:
typedef struct ktudb{
	uint16_t siz;
	uint16_t spc;
	uint16_t flg;
	uint16_t unknown0;
	uint16_t xid0;
	uint16_t xid1;
	uint32_t xid2;
	uint16_t seg;   //uba1
	uint8_t rec;    //uba2
	uint8_t unknown1;
}ktudb;

ktubl(是事务的第一个5.1时):
typedef struct ktubl{
	uint32_t objn;  
	uint32_t objd;
	uint32_t tsb;  //表空间编号，猜测是4字节
	uint32_t unknown1;
	uint8_t opc0;
	uint8_t upc1;
	uint8_t slt;
	uint8_t rci;
	uint32_t unknown2[2];
	uint32_t prev ctl uba0;
	uint16_t prev ctl uba1;
	uint8_t prev ctl uba2;
	uint8_t unknown3;
	uint32_t prev ctl max cmt scn_base;
	uint16_t prev ctl max cmt scn_wrapper;
	uint16_t unknown4;
	uint32_t prev tx cmt scn_base;
	uint16_t prev tx cmt scn_wrapper;
	uint16_t unknown4;
	uint32_t unknown5;
	uint32_t txn start scn_base;
	uint16_t txn start scn_wrapper;
	uint16_t unknown6;
	uint32_t prev brb;
	uint32_t unknown7;
	uint16_t logon user;   //位数不确定，执行这个事务的用户
	uint16_t unknown8;
}ktubl;

ktubu(不是事务的第一个5.1时):

ktubu redo: slt: 25 rci: 5 opc: 11.1 objn: 66450 objd: 66450 tsn: 6
Undo type:  Regular undo       Undo type:  Last buffer split:  No 
Tablespace Undo:  No 
             0x00000000

typedef struct ktubu{
	uint32_t objn;
	uint32_t objd;
	uint32_t tsn;   //表空间编号，位数不确定
	uint32_t unknown0;
	uint8_t opc0;
	uint8_t opc1;
	uint8_t slt;
	uint8_t rci;
	uint32_t unknown1;
}ktubu;

KTB,KDO等这些都是对应11.x操作的相反操作，例如:11.2对应11.3，11.3对应11.2，11.5对应11.5，这里不再赘述，想不通的话，可以联系我。
```

## opcode=5.4,commit操作，表示一个事务的结束

```
CHANGE #3 TYP:0 CLS:21 AFN:3 DBA:0x00c000a0 OBJ:4294967295 SCN:0x0000.000fc635 SEQ:1 OP:5.4 ENC:0 RBL:0

ktucm redo: slt: 0x0001 sqn: 0x00000338 srt: 0 sta: 9 flg: 0x2 
ktucf redo: uba: 0x00c028b0.00ac.01 ext: 26 spc: 8012 fbi: 0 

对于一个5.4操作,结构视情况而定。

ktucm:
typedef struct ktucm{
	uint16_t slt;
	uint16_t unknown0;
	uint32_t sqn;   //xid2
	uint32_t unknown1;
	uint8_t sta;
	uint24_t unknown2;
	uint8_t flg;
	uint24_t unknown3;
}

情况一:
	事务commit后，最后使用的undo块中还有空间可以提交给别的事务使用时，5.4中会记录将undo块提交到空闲池列表中。此时ktucm的flg会等于0x02或0x12。
	此时5.4结构为ktucm-ktucf-未知4字节

typedef struct ktucf{
	uint32_t uba0;
	uint16_t uba1;
	uint8_t uba2;
	uint8_t unknown0;
	uint16_t ext;
	uint16_t spc;
	uint32_t unknown1;
}ktucf;

第三部分是固定的4字节，作用未知。

情况二:
	事务commit后，最后使用的undo块中没有空间给别的事务时，此时5.4的结构为ktucm-未知4字节。此时ktucm的flg参数为0x00或0x10。
	
情况三:
	事务回滚时，5.4的结构为ktucm-未知4字节，此时ktucm的flg参数为0x04或0x14。
```

## 最后

```
	常用的change还有11.11(批量更新),11.12(批量删除),11.19(数组更新)，可以先对11.2，11.3，11.5做大量练习后再看这几个opcode，先占坑，迟点更新。
```

# 计算
&emsp;&emsp;举例11.5的例子，11.5理解之后，11.2和11.3也容易理解了。   

```
CHANGE #1 TYP:2 CLS:1 AFN:1 DBA:0x00414f09 OBJ:73444 SCN:0x0000.000f6832 SEQ:2 OP:11.5 ENC:0 RBL:0
KTB Redo 
op: 0x11  ver: 0x01  
compat bit: 4 (post-11) padding: 1
op: F  xid:  0x0006.00f.00000347    uba: 0x00c001da.00d1.31
Block cleanout record, scn:  0x0000.000fd835 ver: 0x01 opt: 0x02, entries follow...
  itli: 2  flg: 2  scn: 0x0000.000f6832
KDO Op code: URP row dependencies Disabled
  xtype: XA flags: 0x00000000  bdba: 0x00414f09  hdba: 0x00414f08
itli: 1  ispac: 0  maxfr: 4863
tabn: 0 slot: 0(0x0) flag: 0x2c lock: 1 ckix: 0
ncol: 3 nnew: 2 size: -1
col  0: [ 2]  c1 23
col  2: [ 6]  77 65 72 74 79 75

二进制:
[change header]
0b050100 01000100 094f4100 32680f00 00004000 0202e41e 
[change length list]
0c004000 1d000400 02000600 

[第一部分]
110d0000 00000000 06000f00 47030000 da01c000 d1003100 00000000 00000000 
00000000 00000000 00000000 02010100 35d80f00 00000000 02020000 32680f00 

[第二部分]
094f4100 084f4100 ff120501 01000000 2c010000 00000302 ffff0000 00000000 

[第三部分]
00000200 

[字段值]
c1237704 
77657274 79754007

[注意]这里我已经区分了，只取11.5这部分的二进制，如果对结构计算海不熟练的话，需要回去重看redo_struct.md文件。
	从change length list中可以得知,字段编号部分长度为4，值是00000200,计算得到:0, 2，
redo中计算字段编号从0开始，而数据库中从1开始。根据字段编号查询COL$表可以得到字段名称。
	从change length list中可以得知，第一个字段值的长度为2，取值:c1 23，第二个字段值的长度为6，取值:77 65 72 74 79 75。
```
