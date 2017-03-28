[ ![Download](https://api.bintray.com/packages/mingyuan/maven/smoke/images/download.svg?version=2.0.0) ](https://bintray.com/mingyuan/maven/smoke/2.0.0/link)

## Smoke
Smoke 是以一个在 Android 平台上使用的日志封装库，具备以下特性：
### 打印接口简洁
* 相比系统原生的 Log.d(TAG,"message") 打印，Smoke 直接使用默认方法名作为TAG,使开发者专注于日志消息本身；

* 传统的String.format形式，使得我们打印一个消息时往往还要去注意参数的类型，十分恶心;

  为了更懒一点，Smoke 内部自动做了参数的类型判断，并支持 MessageFormat 格式化形式；

#### 支持空消息打印，常用于标识是否有某个逻辑方法是否有调用到；
```
public void doSmokePrint() {
    Smoke.verbose();
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.doSmokePrint(SmokeTest.java:59)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║
╚════════════════════════════════════════════════════════════════════════════════════════
```

#### 支持带消息的正常打印；
```
public void doSmokePrint() {
    Smoke.debug("Hello");
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.doSmokePrint(SmokeTest.java:59)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║Hello
╚════════════════════════════════════════════════════════════════════════════════════════
```

#### 支持标准的日志格式化；
```
public void doSmokePrint() {
    Smoke.debug("Hello，%s","Lilei");
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.doSmokePrint(SmokeTest.java:59)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║Hello,Lilei
╚════════════════════════════════════════════════════════════════════════════════════════
```

#### 支持且推荐使用MessageFormat形式的日志格式化；
```
public void doSmokePrint() {
    Smoke.debug("Hello，{}","Hanmeimei");
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.doSmokePrint(SmokeTest.java:59)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║Hello,Hanmeimei
╚════════════════════════════════════════════════════════════════════════════════════════
```
```
public void doSmokePrint() {
    Smoke.debug("Hello，{0} and {1}","Lilei","Hanmeimei");
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.doSmokePrint(SmokeTest.java:59)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║Hello,Lilei and Hanmeimei
╚════════════════════════════════════════════════════════════════════════════════════════
```
#### 支持异常堆栈打印
```
public void doSmokePrint() {
    Throwable throwable = new Throwable();
    Smoke.debug("error message.",throwable);
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.doSmokePrint(SmokeTest.java:75)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║error message.
╟────────────────────────────────────────────────────────────────────────────────────────
║java.lang.Throwable
║	at com.mingyuans.smoke.SmokeTest.doSmokePrint(SmokeTest.java:74)
║	at java.lang.reflect.Method.invoke(Native Method)
║	at org.junit.runners.model.FrameworkMethod$1.runReflectiveCall(FrameworkMethod.java:50)
║	at ................
╚════════════════════════════════════════════════════════════════════════════════════════
```

#### 支持XML/JSON自动判断并格式化打印
```
public void doSmokePrint() {
    String xmlString = "<team><member name=\"Elvis\"/><member name=\"Leon\"/></team>";
    Smoke.info(xmlString);
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.doSmokePrint(SmokeTest.java:132)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║<?xml version="1.0" encoding="UTF-8"?>
║<team>
║  <member name="Elvis"/>
║  <member name="Leon"/>
║</team>
╚════════════════════════════════════════════════════════════════════════════════════════
```
```
public void testJson() {
    String json = "{\"code\":\"1\",\"content\":\"hello\"}";
    Smoke.warn(json);
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.testJson(SmokeTest.java:125)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║{
║  "code": "1",
║  "content": "hello"
║}
╚════════════════════════════════════════════════════════════════════════════════════════
```

#### 支持数组直接打印
```
public void testFormat() {
    String[] array = new String[]{"Hello","World"};
    Smoke.debug("array is : {0}",(Object)array);
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.testFormat(SmokeTest.java:101)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║array is : [Hello,World]
╚════════════════════════════════════════════════════════════════════════════════════════
```

#### 支持长日志打印
```
public void testTooLongMessage() {
    final int MAX_LINE_LENGTH = 4000;
    StringBuilder builder = new StringBuilder();
    int index = 0;
    while (builder.length() < (MAX_LINE_LENGTH + 200)) {
    builder.append("[" + index++ + "]");
    }
    Smoke.debug("last index : " + (index-1));
    Smoke.debug(builder.toString());
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║[SmokeTest.testTooLongMessage(SmokeTest.java:113)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║[0][1][2][3][4][5][6][7][8][9][10][11][12][13][14][15][16][17][18][19][20][21][22][23][24][25][26][27][28][29][30][31][32][33][34][35][36][37][38][39][40][41][42][43][44][45][46][47][48][49][50][51][52][53][54][55][56][57][58][59][60][61][62][63][64][65][66][67][68][69][70][71][72][73][74][75][76][77][78][79][80][81][82][83][84][85][86][87][88][89][90][91][92][93][94][95][96][97][98][99][100][101][102][103][104][105][106][107][108][109][110][111][112][113][114][115][116][117][118][119][120][121][122][123][124][125][126][127][128][129][130][131][132][133][134][135][136][137][138][139][140][141][142][143][144][145][146][147][148][149][150][151][152][153][154][155][156][157][158][159][160][161][162][163][164][165][166][167][168][169][170][171][172][173][174][175][176][177][178][179][180][181][182][183][184][185][186][187][188][189][190][191][192][193][194][195][196][197][198][199][200][201][202][203][204][205][206][207][208][209][210][211][212][213][214][215][216][217][218][219][220][221][222][223][224][225][226][227][228][229][230][231][232][233][234][235][236][237][238][239][240][241][242][243][244][245][246][247][248][249][250][251][252][253][254][255][256][257][258][259][260][261][262][263][264][265][266][267][268][269][270][271][272][273][274][275][276][277][278][279][280][281][282][283][284][285][286][287][288][289][290][291][292][293][294][295][296][297][298][299][300][301][302][303][304][305][306][307][308][309][310][311][312][313][314][315][316][317][318][319][320][321][322][323][324][325][326][327][328][329][330][331][332][333][334][335][336][337][338][339][340][341][342][343][344][345][346][347][348][349][350][351][352][353][354][355][356][357][358][359][360][361][362][363][364][365][366][367][368][369][370][371][372][373][374][375][376][377][378][379][380][381][382][383][384][385][386][387][388][389][390][391][392][393][394][395][396][397][398][399][400][401][402][403][404][405][406][407][408][409][410][411][412][413][414][415][416][417][418][419][420][421][422][423][424][425][426][427][428][429][430][431][432][433][434][435][436][437][438][439][440][441][442][443][444][445][446][447][448][449][450][451][452][453][454][455][456][457][458][459][460][461][462][463][464][465][466][467][468][469][470][471][472][473][474][475][476][477][478][479][480][481][482][483][484][485][486][487][488][489][490][491][492][493][494][495][496][497][498][499][500][501][502][503][504][505][506][507][508][509][510][511][512][513][514][515][516][517][518][519][520][521][522][523][524][525][526][527][528][529][530][531][532][533][534][535][536][537][538][539][540][541][542][543][544][545][546][547][548][549][550][551][552][553][554][555][556][557][558][559][560][561][562][563][564][565][566][567][568][569][570][571][572][573][574][575][576][577][578][579][580][581][582][583][584][585][586][587][588][589][590][591][592][593][594][595][596][597][598][599][600][601][602][603][604][605][606][607][608][609][610][611][612][613][614][615][616][617][618][619][620][621][622][623][624][625][626][627][628][629][630][631][632][633][634][635][636][637][638][639][640][641][642][643][644][645][646][647][648][649][650][651][652][653][654][655][656][657][658][659][660][661][662][663][664][665][666][667][668][669][670][671][672][673][674][675][676][677][678][679][680][681][682][683][684][685][686][687][688][689][690][691][692][693][694][695][696][697][698][699][700][701][702][703][704][705][706][707][708][709][710][711][712][713][714][715][716][717][718][719][720][721][722][723][724][725][726][727][728][729][730][731][732][733][734][735][736][737][738][739][740][741][742][743][744][745][746][747][748][749][750][751][752][753][754][755][756][757][758][759][760][761][762][763][764][765][766][767][768][769][770][771][772][773][774][775][776][777][778][779][780][781]
║[782][783][784][785][786][787][788][789][790][791][792][793][794][795][796][797][798][799][800][801][802][803][804][805][806][807][808][809][810][811][812][813][814][815][816][817][818][819][820][821][822][823][824][825][826][827][828][829][830][831][832][833][834][835][836][837][838][839][840][841][842][843][844][845][846][847][848][849][850][851][852][853][854][855][856][857][858][859][860][861]
╚════════════════════════════════════════════════════════════════════════════════════════
```

### 支持多级TAG 和 日志配置继承关系

多级 TAG 用于帮助区分来着不同模块的日志消息；
newSub 方法得到的子对象将继承父对象的日志配置；
```
public void testNewSub() {
    SubSmoke subSmoke = Smoke.newSub("subOne");
    subSmoke.debug("Hello, subOne!");
    SubSmoke subSmoke1 = subSmoke.newSub("subTwo");
    subSmoke1.debug("Hello, {0}!","subTwo");
}
//Output:
╔════════════════════════════════════════════════════════════════════════════════════════
║【subOne】[SmokeTest.testNewSub(SmokeTest.java:151)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║Hello, subOne!
╚════════════════════════════════════════════════════════════════════════════════════════
    
╔════════════════════════════════════════════════════════════════════════════════════════
║【subOne|subTwo】[SmokeTest.testNewSub(SmokeTest.java:153)][thread: main]
╟────────────────────────────────────────────────────────────────────────────────────────
║Hello, subTwo!
╚════════════════════════════════════════════════════════════════════════════════════════

```

### 支持 AndroidStudio 中代码定位
点击日志中高亮的文字即可让 AndrodiStudio 跳转到该日志打印位置；
![image](http://ocrfgcvcm.bkt.clouddn.com/smoke_jump.png)

### 支持日志等级过滤
```
Smoke.setLogPriority(Smoke.INFO);
```

### 支持日志流程自定义
Smoke 使用链式递归调用的设计来处理日志的修饰和输出并开放了流程配置接口，开发者可以根据自己的需要进行日志处理流程的修改；
```
public void testProcessDIY() throws Exception {
   SubSmoke subSmoke = new SubSmoke(InstrumentationRegistry.getContext(),"Smoke");
   String drawBoxIdentify = Processes.getIdentify(DrawBoxProcess.class);
   subSmoke.getProcesses()
      .removeByIdentify(drawBoxIdentify) //If you do not like box!
      .addCollector(new MyDIYLogCollector())
      .addPrinter(new MyDIYLogPrinter());
   subSmoke.debug("Hello");
}

public static class MyDIYLogCollector extends Smoke.Process {
   @Override
   public boolean proceed(Smoke.LogBean logBean, List<String> messages, Chain chain) {
       LinkedList<String> newMessages = new LinkedList<>();
       for (String message: messages) {
       newMessages.add("Test" + message);
       }
       return chain.proceed(logBean,newMessages);
   }
}

public static class MyDIYLogPrinter extends Smoke.Process {

   @Override
   public boolean proceed(Smoke.LogBean logBean, List<String> messages, Chain chain) {
       for (String message: messages) {
       Log.i("SmokeDIY",message);
       }
       return chain.proceed(logBean,messages);
   }
}
```

**想要日志特殊过滤？想要去除框框？想要加前缀？想要输出到其他控制台？
统统可以通过增删 Process 来实现！**


### 支持SDK场景运用下的日志控制
当我们在开发某个 SDK 时，不可避免的，我们会需要对外提供一个接口，让SDK 使用者可以控制我们 SDK 内的日志打印，方便进行调试；

Smoke 自带供外部设置的 Printer，
更进一步，如果调用方也使用 Smoke 模块，还可以实现 SDK 内 Smoke 日志实例挂载到对方日志实例上，实现日志输出格式的统一；

```

public void testAttachPrinter() throws Exception {
    SubSmoke subSmoke = new SubSmoke(InstrumentationRegistry.getContext(),"Smoke",null);
    subSmoke.attach(new Printer() {
    @Override
    public void println(int priority, String tag, String message) {
        Log.println(priority,tag,"Smoke Printer: Hello");
        Log.println(priority,tag,message);
    }
    });
    subSmoke.debug("Hello, Printer!");
}
```

```
public void testAttachSub() throws Exception {
    SubSmoke subSmoke = new SubSmoke(InstrumentationRegistry.getContext(),"SmokeParent",null);
    subSmoke.getProcesses().addCollector(new Smoke.Process() {
        @Override
        public boolean proceed(Smoke.LogBean logBean, List<String> messages, Chain chain) {
            messages.add(0,"Smoke A: Hello, Smoke B.");
            return chain.proceed(logBean,messages);
        }
    });
    SubSmoke subSmoke1 = new SubSmoke(InstrumentationRegistry.getContext(),"smokeChild",null);
    subSmoke1.attach(subSmoke);
    subSmoke1.info("Hello,Smoke.");
}
```

### 自带接口和实现分离，避免后期更换动骨
对于第三方的东西，总是要保持着怀疑和不信任去使用；
Smoke 身为一个需要埋点到 App 中诸多地方的一个第三方开源库，一旦有 Bug 发生和后期更换需求，替换其他日志模块将是一个浩大的工程；

为了解决这个问题，Smoke 设计之时也为开发者做好了解决方案：
1. Smoke 库分 Smoke 和 SubSmoke 两部分，Smoke 本身为空接口类，SubSmoke 才是具体实现；
   对于 App 场景，开发者直接使用 Smoke 类来进行日志打印；至于其核心 SubSmoke 实例则可以随时更换；

2. SubSmoke 作为日志打印的具体实现，其内部本身也没有太多逻辑，其主要核心实现均依赖于多个 Process 的组合配置。
   如果实践中发现某个 Process 有Bug，开发者完全可以移除或修改该 Process，不需要等等官方修改发布新的版本；

综上两个方案，能最大化降低 Smoke 因本身 Bug 或后期不适用需要大规模替换的情况，从而降低开发者使用上的风险；

### 异常管控
Smoke 本身逻辑较多，在前期测试有限的情况下难以保证稳定性；为了降低试错成本，Smoke也自带了一个自身异常后自动禁用的小策略;

## Licence  
``` 
Copyright 2016-2017 Mingyuans

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

```







