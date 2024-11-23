// #include "node.h"
// #include <gtest/gtest.h>

// TEST(NodeTest, Init)
// {
//     Node node(1);
//     node.init();
//     // 可以添加一些断言来检查初始化是否成功
// }

// TEST(NodeTest, InitUDPSocket)
// {
//     Node node(1);
//     int result = node.init_udp_socket();
//     EXPECT_EQ(result, 0);
//     // 可以进一步检查套接字的属性等
// }

// TEST(NodeTest, Multiacast)
// {
//     Node node(1);
//     node.init();
//     std::vector<uint8_t> bytes = {'a', 'b', 'c'};
//     int result;
//     for (int i = 0; i < 10000000; i++)
//     {
//          result = node.multiacast(bytes);
//     }
//     EXPECT_EQ(result, 0);
//     // 可以检查是否成功发送了数据
// }

// // TEST(NodeTest, Create) {
// //     Node node(1);
// //     std::shared_ptr<Topic> topic = std::make_shared<Topic>("TestTopic");
// //     bool result = node.create("TestTopic", topic);
// //     EXPECT_TRUE(result);
// //     // 可以检查主题是否成功创建
// // }

// // TEST(NodeTest, Publish) {
// //     Node node(1);
// //     std::shared_ptr<Topic> topic = std::make_shared<Topic>("TestTopic");
// //     bool result = node.publish("TestTopic", topic);
// //     EXPECT_TRUE(result);
// //     // 可以检查是否成功发布了消息
// // }

// // TEST(NodeTest, Subscribe) {
// //     Node node(1);
// //     bool result = node.subscribe("TestTopic");
// //     EXPECT_TRUE(result);
// //     // 可以检查是否成功订阅了主题
// // }
