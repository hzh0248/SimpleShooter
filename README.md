SimpleShooter 是一个基于 Unreal Engine 5（C++ + Blueprint） 的多人射击 Demo 项目。


本地联机测试方式
1、Editor内可通过play as listen server模式测试。
2、打包成exe后，启动游戏，主界面选择Host Game作为listen server;其他客户端选择Join Game。

打包设置
1、本项目包含两张地图，主菜单Lvl_MainMenu和游戏地图Lvl_Shooter。
2、在项目设置中将Lvl_MainMenu作为默认地图。

游戏规则
1、游戏为PvPvE，玩家各自为战，场中有AI进行干扰。AI会攻击玩家，玩家也可以击杀AI。
2、击杀Player得3分，击杀AI得1分，当某个玩家分数达到10分时，游戏结束。
3、游戏结束后，点击退出游戏可关闭游戏。

按键设置
W、A、S、D 		移动
空格				跳跃
鼠标左键  		射击
Shift			切换武器
Tab				查看分数排行

