<<<<<<< Updated upstream
# stm32f4xx-ucosii
=======
<<<<<<< HEAD
<!--
 * @Author: your name
 * @Date: 2020-06-12 14:06:13
 * @LastEditTime: 2020-06-15 18:39:03
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f4xx-ucosii\README.md
--> 
# stm32f4xx-ucosii
1. 增加环形缓冲区用于串行通信；
2. 移植CycloneTCP到ucosii系统上；
3. CycloneTCP协议栈移植成功，电脑可以ping通开发板，需要优化malloc函数；
4. 增加MQTT连接阿里云平台；
5. 使用MQTT时要根据自己的平台参数进行配置，mqtt_client.h,出现202错误是由于username的缓冲区太小导致错误；
=======
# stm32f4xx-ucosii
移植ucosii在STM32F407的平台运行；
>>>>>>> master
>>>>>>> Stashed changes
