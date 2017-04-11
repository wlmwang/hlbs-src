# 权重计算

* 各Svr节点负载W(k)计算，k为某一Svr节点，0…n某一(GID,XID)所对应的所有节点：

```
W(k) = delay_load(k) * ok_load(k) * weight_load(k) * f(rate_weight) * f(delay_weight)

delay_load (k) = delay(k)/min(delay(0)…delay(n))    #取值范围：1~100000000（100s）

ok_load(k) = max(ok_rate(0)…ok_rate(n))/ ok_rate(k) #取值范围：1~1000000

weight_load(k) = max(weight(0)…weight(n))/weight(k) #取值范围：weight(n)= 0~1000,其中0为删除该Svr

f(rate_weight)、 f(delay_weight)为在配置文件（qos.xml）中配置数值。 #取值范围：f(rate_weight)/f(delay_weight)= 0.01~100
```

