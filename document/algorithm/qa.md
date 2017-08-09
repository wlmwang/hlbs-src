# 算法关键QA
1. 什么情况下节点会出现过载或宕机？
    * 当节点本周期的请求量超过门限，并错误率超过预设最低错误率时，判定该节点过载。
    * 当节点门限收缩到预设最小请求量以下，判断该节点为宕机。

2. 什么情况下获取路由会失败？
    * 当所有节点本周期请求量都超过了门限，并错误率也都超过预设最低错误率。
    * 当所有节点门限都收缩到预设最小请求量以下。
    * 所有节点都是远程节点。

3. 什么情况下一个节点会进入宕机列表？
    * 当一节点门限收缩到预设最小请求量以下。

4. 什么情况下一个宕机节点会进入PING及TCP探测并尝试拉起该节点？
    * 当一节点宕机后，若该路由的请求量超过预设的最大宕机请求量时，将进行宕机探测。
    * 节点宕机3s后，进行宕机探测。
    * 注意：宕机列表节点一旦进入宕机探测，将一直探测，直到该路由恢复为止。

5. 当某一周期节点错误率超过了预设最小错误率时，为什么不返回该节点过载错误？
    * 这样处理可使：某被踢节点在刚恢复情况下，即使门限较低，只要错误率低于预设的错误率阀值，也仍然可以分配该节点。

6. 节点有效错误率相比节点实际错误率有什么不同，为什么要存在该参数指标？
    * 节点有效错误率=节点实际错误率-所有节点平均错误率，即某一节点相比总体节点平均错误率误差。该指标主要来区分：当某一节点错误率居高不下时，有可能并不是该节点问题，很可能是所有节点平均错误率都很高~。该指标主要用户门限扩缩逻辑。

7. 路由重建时间间隔选取应考虑哪些因素？
    * 路由重建时间间隔，即为节点统计周期。不能太短、也不能太长，默认1分钟（建议配置为10s~1min）。因为：
    * 若周期太短：则节点统计出的负载值、成功率、错误率、平均延时等数据参考性就不高，甚至会带有偶然性。某一网络波动，都可能会很大程度上影响节点统计数据。其次，路由重建的逻辑是由客户端某一请求激活，太短周期会让客户端请求频繁进行重建路由，不够友好。
    * 若周期太长：则服务器对某节点的压力故障、拒绝服务或其他服务宕机时，显得“反应太慢”。
    * 服务器为了防止在时间跳变（可能硬件时钟故障或人为修改服务器时间等）情况下仍可正常工作，在每次重建路由前都会判定：若当前时间距离上次重建时间间隔的绝对值大于2倍周期长度，将不会重建路由。这也导致了，若周期太短，或请求太少（2个周期以上没有请求）将不会使路由重建。