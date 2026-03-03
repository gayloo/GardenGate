# Launch args

Here you can find launch arguements with their examples/defaults.

Please keep in mind some args here are tied to the `dinput8.dll`

### Universal
```
-name Player
-dataPath ModData/Default
-Client.ServerIp 127.0.0.1:25200
-listen 0.0.0.0:25200
-Server.ServerPassword 123456
-Window.Fullscreen true
-GameTime.MaxSimFps 60
```

### GW1
```
-level _pvz/Levels/Coastal/Level_COOP_Coastal/Level_COOP_Coastal
-Game.DefaultLayerInclusion GameMode=Coop0
-Network.ServerPort 25200
-PerfOverlay.DrawFps false
-PVZServer.InActivityTimeOut 180
-SyncedBFSettings.AllUnlocksUnlocked true
```

### GW2
```
-level Levels/Level_FE_Hub/Level_FE_Hub
-Game.DefaultLayerInclusion GameMode=FreeRoam
-Network.ServerPort 25200
-PerfOverlay.DrawFps false
-PVZServer.InActivityTimeOut 180
-GameMode.SkipIntroHubNIS false
-Online.OnlineGameInteractionMasterKillSwitch true
-SyncedBFSettings.AllUnlocksUnlocked false
```

### BFN
```
-GameSettings.InitialDSubLevel Levels/DSub_SocialSpace/DSub_SocialSpace
-GameSettings.StartPoint StartPoint_SocialSpace
-GameSettings.DefaultLayerInclusion GameMode=Mode_SocialSpace;HostedMode=PeerHosted
-NetObjectSystem.MaxServerConnectionCount 4
-GameModeSettings.ForceHUBSeason 0
```
