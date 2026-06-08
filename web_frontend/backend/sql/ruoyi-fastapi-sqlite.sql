/*
 Navicat Premium Data Transfer

 Source Server         : sqlite3
 Source Server Type    : SQLite
 Source Server Version : 3021000
 Source Schema         : main

 Target Server Type    : SQLite
 Target Server Version : 3021000
 File Encoding         : 65001

 Date: 04/01/2021 14:54:16
*/

PRAGMA
foreign_keys = false;

-- ----------------------------
-- Table structure for dq_app_user
-- ----------------------------
DROP TABLE IF EXISTS "dq_app_user";
CREATE TABLE "dq_app_user"
(
    "app_user_id"    integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "username"       text(20) NOT NULL,
    "password"       text(64) NOT NULL,
    "account_status" integer(11) NOT NULL,
    "truename"       text(20),
    "money"          real(10,2) NOT NULL,
    "phone"          text(11) NOT NULL,
    "wx"             text(20),
    "create_date"    text    NOT NULL,
    "create_by"      text(50) NOT NULL,
    "update_date"    text,
    "update_by"      text(50),
    "role_id"        integer(11) NOT NULL,
    "yqm"            text(50),
    "rechargeamount" real(10,2),
    "rewardamount"   real(10,2),
    "rewardbalance"  real(10,2)
);

-- ----------------------------
-- Table structure for gen_table
-- ----------------------------
DROP TABLE IF EXISTS "gen_table";
CREATE TABLE "gen_table"
(
    "table_id"        integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "table_name"      text(200),
    "table_comment"   text(500),
    "class_name"      text(100),
    "tpl_category"    text(200),
    "package_name"    text(100),
    "module_name"     text(30),
    "business_name"   text(30),
    "function_name"   text(50),
    "function_author" text(50),
    "options"         text(1000),
    "create_by"       text(64),
    "create_time"     text,
    "update_by"       text(64),
    "update_time"     text,
    "remark"          text(500)
);

-- ----------------------------
-- Records of gen_table
-- ----------------------------
INSERT INTO "gen_table"
VALUES (1, 'dq_account_record', '', 'DqAccountRecord', 'crud', 'com.ruoyi.project.system', 'system', 'record', NULL,
        'ruoyi', NULL, 'admin', '2020-06-20 09:56:17', '', NULL, NULL);
INSERT INTO "gen_table"
VALUES (2, 'dq_marking_record', '打标记录表', 'DqMarkingRecord', 'crud', 'com.ruoyi.project.system', 'system', 'record',
        '打标记录', 'ruoyi', NULL, 'admin', '2020-06-20 09:56:17', '', NULL, NULL);
INSERT INTO "gen_table"
VALUES (3, 'dq_pay_order', '', 'DqPayOrder', 'crud', 'com.ruoyi.project.system', 'system', 'order', NULL, 'ruoyi', NULL,
        'admin', '2020-06-20 15:44:43', '', NULL, NULL);
INSERT INTO "gen_table"
VALUES (4, 'dq_pdt_flow', '', 'DqPdtFlow', 'crud', 'com.ruoyi.project.system', 'system', 'flow', NULL, 'ruoyi', NULL,
        'admin', '2020-06-23 19:40:01', '', NULL, NULL);
INSERT INTO "gen_table"
VALUES (5, 'dq_call_record', '接口调用记录', 'DqCallRecord', 'crud', 'com.ruoyi.project.system', 'system', 'record',
        '接口调用记录', 'lur', '{}', 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43', NULL);

-- ----------------------------
-- Table structure for gen_table_column
-- ----------------------------
DROP TABLE IF EXISTS "gen_table_column";
CREATE TABLE "gen_table_column"
(
    "column_id"      integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "table_id"       text(64),
    "column_name"    text(200),
    "column_comment" text(500),
    "column_type"    text(100),
    "java_type"      text(500),
    "java_field"     text(200),
    "is_pk"          text(1),
    "is_increment"   text(1),
    "is_required"    text(1),
    "is_insert"      text(1),
    "is_edit"        text(1),
    "is_list"        text(1),
    "is_query"       text(1),
    "query_type"     text(200),
    "html_type"      text(200),
    "dict_type"      text(200),
    "sort"           integer(11),
    "create_by"      text(64),
    "create_time"    text,
    "update_by"      text(64),
    "update_time"    text
);

-- ----------------------------
-- Records of gen_table_column
-- ----------------------------
INSERT INTO "gen_table_column"
VALUES (1, 1, 'ar_id', '账户流水记录主键', 'bigint(20)', 'Long', 'arId', 1, 1, NULL, 1, NULL, NULL, NULL, 'EQ', 'input',
        '', 1, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (2, 1, 'app_user_id', '用户ID', 'bigint(20)', 'Long', 'appUserId', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 2,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (3, 1, 'amount', '交易金额', 'decimal(10,2)', 'Double', 'amount', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 3,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (4, 1, 'old_amount', '原金额', 'decimal(10,2)', 'Double', 'oldAmount', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 4,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (5, 1, 'new_amount', '现金额', 'decimal(10,2)', 'Double', 'newAmount', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 5,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (6, 1, 'record_type', '交易类型（1=支付宝充值、2=线下充值、3=后台扣款）', 'varchar(3)', 'String', 'recordType', 0, 0,
        1, 1, 1, 1, 1, 'EQ', 'select', '', 6, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (7, 1, 'name', '消费者', 'varchar(50)', 'String', 'name', 0, 0, NULL, 1, 1, 1, 1, 'LIKE', 'input', '', 7,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (8, 1, 'remark', '交易备注', 'varchar(250)', 'String', 'remark', 0, 0, NULL, 1, 1, 1, NULL, 'EQ', 'input', '', 8,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (9, 1, 'create_date', '创建时间', 'datetime', 'Date', 'createDate', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'datetime', '', 9,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (10, 1, 'create_by', '创建人', 'varchar(50)', 'String', 'createBy', 0, 0, 1, 1, NULL, NULL, NULL, 'EQ', 'input',
        '', 10, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (11, 2, 'marking_id', '打标id', 'bigint(50)', 'Long', 'markingId', 1, 1, NULL, 1, NULL, NULL, NULL, 'EQ',
        'input', '', 1, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (12, 2, 'app_user_id', '用户id', 'bigint(50)', 'Long', 'appUserId', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 2,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (13, 2, 'tb_commodity_id', '淘宝商品id', 'bigint(50)', 'Long', 'tbCommodityId', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'input', '', 3, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (14, 2, 'keyword', '关键字', 'varchar(100)', 'String', 'keyword', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 4,
        'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (15, 2, 'marking_type', '打标类型（0:单次打标记录,1:批量打标记录,2:抢对手打标记录）', 'varchar(2)', 'String',
        'markingType', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'select', '', 5, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (16, 2, 'commodity_url', NULL, 'varchar(5000)', 'String', 'commodityUrl', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'textarea', '', 6, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (17, 2, 'wangwang_id', '旺旺id', 'varchar(100)', 'String', 'wangwangId', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input',
        '', 7, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (18, 2, 'create_date', '创建时间', 'datetime', 'Date', 'createDate', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'datetime', '',
        8, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (19, 2, 'update_date', '修改时间', 'datetime', 'Date', 'updateDate', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'datetime', '',
        9, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (20, 2, 'status', '打标状态（0成功，1失败）', 'varchar(2)', 'String', 'status', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'radio', '', 10, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (21, 2, 'failed_cause', '失败原因', 'varchar(100)', 'String', 'failedCause', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'input', '', 11, 'admin', '2020-06-20 09:56:17', '', NULL);
INSERT INTO "gen_table_column"
VALUES (22, 3, 'pr_id', '支付订单主键', 'bigint(20)', 'Long', 'prId', 1, 1, NULL, 1, NULL, NULL, NULL, 'EQ', 'input',
        '', 1, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (23, 3, 'app_user_id', '会员ID', 'bigint(20)', 'Long', 'appUserId', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 2,
        'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (24, 3, 'order_no', '订单号', 'varchar(50)', 'String', 'orderNo', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 3,
        'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (25, 3, 'amount', '支付金额', 'decimal(16,2)', 'Double', 'amount', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 4,
        'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (26, 3, 'pay_type', '支付类型(10=在线支付、20=线下支付)', 'varchar(3)', 'String', 'payType', 0, 0, 1, 1, 1, 1, 1,
        'EQ', 'select', '', 5, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (27, 3, 'pay_platform', '支付平台(wxpay=微信充值、alipay=支付宝充值、online=手动充值)', 'varchar(10)', 'String',
        'payPlatform', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 6, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (28, 3, 'pay_status', '支付状态(10=待支付、20=支付失败、30=支付成功、40=已关闭、50=已取消)', 'varchar(3)', 'String',
        'payStatus', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'radio', '', 7, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (29, 3, 'recharge_type', '0=老的，1=对冲，2=漏充，3=补偿亏损', 'varchar(3)', 'String', 'rechargeType', 0, 0, NULL,
        1, 1, 1, 1, 'EQ', 'select', '', 8, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (30, 3, 'currency', 'CNY', 'varchar(10)', 'String', 'currency', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 9,
        'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (31, 3, 'third_order_no', '第三方支付订单号', 'varchar(50)', 'String', 'thirdOrderNo', 0, 0, NULL, 1, 1, 1, 1,
        'EQ', 'input', '', 10, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (32, 3, 'create_date', '创建时间', 'datetime', 'Date', 'createDate', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'datetime', '',
        11, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (33, 3, 'update_by', '最后更新人', 'varchar(50)', 'String', 'updateBy', 0, 0, NULL, 1, 1, NULL, NULL, 'EQ',
        'input', '', 12, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (34, 3, 'update_date', '最后更新时间', 'datetime', 'Date', 'updateDate', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'datetime', '', 13, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (35, 3, 'pay_desc', '支付错误信息描述', 'varchar(200)', 'String', 'payDesc', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'input', '', 14, 'admin', '2020-06-20 15:44:43', '', NULL);
INSERT INTO "gen_table_column"
VALUES (36, 4, 'flow_id', '发布任务id', 'bigint(50)', 'Long', 'flowId', 1, 1, NULL, 1, NULL, NULL, NULL, 'EQ', 'input',
        '', 1, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (37, 4, 'app_user_id', '用户id', 'bigint(50)', 'Long', 'appUserId', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 2,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (38, 4, 'task_type', '任务主类型', 'bigint(255)', 'Long', 'taskType', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'select', '',
        3, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (39, 4, 'task_sub_type', '任务子类型', 'bigint(255)', 'Long', 'taskSubType', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'select', '', 4, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (40, 4, 'api_task_id', '任务索引id', 'bigint(20)', 'Long', 'apiTaskId', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input',
        '', 5, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (41, 4, 'start_date', NULL, 'datetime', 'Date', 'startDate', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'datetime', '', 6,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (42, 4, 'end_date', NULL, 'datetime', 'Date', 'endDate', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'datetime', '', 7,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (43, 4, 'goods_url', '商品地址,实际任务目标商品', 'varchar(1550)', 'String', 'goodsUrl', 0, 0, NULL, 1, 1, 1, 1,
        'EQ', 'textarea', '', 8, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (44, 4, 'keyword_hour',
        '关键词对应的发布时段，多个时段用英文逗号分隔,例如: 0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1,0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_1_2',
        'varchar(1550)', 'String', 'keywordHour', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'textarea', '', 9, 'admin',
        '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (45, 4, 'title', '任务标题,传空则根据日期生成', 'varchar(255)', 'String', 'title', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'input', '', 10, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (46, 4, 'keyword',
        '任务子类型为 6, 9, 13, 49时,此参数不能为空,搜索关键词，多个关键词用英文逗号分隔，例如: 关键词1,关键词2',
        'varchar(255)', 'String', 'keyword', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 11, 'admin',
        '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (47, 4, 'tao_category', '人群标签定位id ,多个可用英文逗号分隔 例如 :', 'varchar(255)', 'String', 'taoCategory',
        0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 12, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (48, 4, 'browse_goods_time_id', '浏览任务商品时间id,不传默认为1', 'bigint(20)', 'Long', 'browseGoodsTimeId', 0,
        0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 13, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (49, 4, 'browse_other_goods', '需要深度浏览其他商品时,不传默认为1,', 'bigint(255)', 'Long', 'browseOtherGoods',
        0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 14, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (50, 4, 'from_goods_url', '其他店铺商品详情地址,当 子任务类型为 54,67,77,83 不能为空,', 'varchar(255)', 'String',
        'fromGoodsUrl', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 15, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (51, 4, 'weitao_url', '微淘地址,任务主类型为17 或 任务子类型为 52, 65, 75, 81, 61 不能为空,参考 ',
        'varchar(255)', 'String', 'weitaoUrl', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 16, 'admin',
        '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (52, 4, 'tao_pwd', '淘口令,任务主类型为4 或 任务子类型为92, 93, 94, 95', 'varchar(255)', 'String', 'taoPwd', 0,
        0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 17, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (53, 4, 'ju_url', '聚划算地址,任务子类型为 12, 64, 70, 86, 102 不能为空, ', 'varchar(255)', 'String', 'juUrl', 0,
        0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 18, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (54, 4, 'reason_tag',
        '商品需要被推荐的标签,英文逗号分隔,当任务子类型为 107 时,此参数不能为空 通过 商品推荐理由 获取,例如: 泡泡袖,简约风穿搭,短袖,波点控,中裙,气质小仙女,简约,圆领,高腰,',
        'varchar(255)', 'String', 'reasonTag', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 19, 'admin',
        '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (55, 4, 'is_sync_yangtao', '当任务子类型为 107 时,商品推荐任务此参数才有意义,是否同步到洋淘, ', 'bigint(255)',
        'Long', 'isSyncYangtao', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 20, 'admin', '2020-06-23 19:40:01', '',
        NULL);
INSERT INTO "gen_table_column"
VALUES (56, 4, 'network_model', '网络模式, 1：随机,2：WIFI, 3：4G,4：2G/3G,不传则为1', 'varchar(255)', 'String',
        'networkModel', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 21, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (57, 4, 'man_ratio', '男用户比率，男都为0表示随机', 'bigint(255)', 'Long', 'manRatio', 0, 0, NULL, 1, 1, 1, 1,
        'EQ', 'input', '', 22, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (58, 4, 'woman_ratio', '女用户比率，男都为0表示随机', 'bigint(255)', 'Long', 'womanRatio', 0, 0, NULL, 1, 1, 1, 1,
        'EQ', 'input', '', 23, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (59, 4, 'tao_credit_level', '任务主类型为 48时,不传默认为1', 'bigint(255)', 'Long', 'taoCreditLevel', 0, 0, NULL,
        1, 1, 1, 1, 'EQ', 'input', '', 24, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (60, 4, 'is_view_goods_comment', '是否查看目标商品评论，0 否 , 1 是 ,不传默认为0', 'bigint(255)', 'Long',
        'isViewGoodsComment', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 25, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (61, 4, 'task_schedule', '任务进度', 'varchar(100)', 'String', 'taskSchedule', 0, 0, 1, 1, 1, 1, 1, 'EQ',
        'input', '', 26, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (62, 4, 'task_status', '任务状态', 'varchar(100)', 'String', 'taskStatus', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'radio',
        '', 27, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (63, 4, 'update_date', '修改时间', 'datetime', 'Date', 'updateDate', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'datetime', '',
        28, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (64, 4, 'failed_cause', '失败原因', 'varchar(100)', 'String', 'failedCause', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'input', '', 29, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (65, 4, 'task_count', '任务数量', 'int(11)', 'Long', 'taskCount', 0, 0, 1, 1, 1, 1, 1, 'EQ', 'input', '', 30,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (66, 4, 'costs', '成本', 'decimal(10,2)', 'Double', 'costs', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 31,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (67, 4, 'money', '消费金币', 'decimal(10,2)', 'Double', 'money', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 32,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (68, 4, 'profit', '利润', 'decimal(10,2)', 'Double', 'profit', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 33,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (69, 4, 'amount', '发放量', 'decimal(10,0)', 'Long', 'amount', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 34,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (70, 4, 'status', '0已创建1进行中2已完成3已关闭', 'varchar(2)', 'String', 'status', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'radio', '', 35, 'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (71, 4, 'jcsj', '监察时间', 'datetime', 'Date', 'jcsj', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'datetime', '', 36,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (72, 4, 'isjc', '是否监察1是0否', 'varchar(2)', 'String', 'isjc', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 37,
        'admin', '2020-06-23 19:40:01', '', NULL);
INSERT INTO "gen_table_column"
VALUES (73, 5, 'call_id', '主键id', 'bigint(20)', 'Long', 'callId', 1, 0, NULL, 1, NULL, NULL, NULL, 'EQ', 'input', '',
        1, 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (74, 5, 'interface_addr', '接口地址', 'varchar(255)', 'String', 'interfaceAddr', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'input', '', 2, 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (75, 5, 'interface_params', '接口参数', 'varchar(1000)', 'String', 'interfaceParams', 0, 0, NULL, 1, 1, 1, 1,
        'EQ', 'textarea', '', 3, 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (76, 5, 'call_time', '调用时间', 'datetime', 'Date', 'callTime', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'datetime', '', 4,
        'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (77, 5, 'call_ip', '调用IP', 'varchar(255)', 'String', 'callIp', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '', 5,
        'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (78, 5, 'call_state', '调用状态', 'varchar(2)', 'String', 'callState', 0, 0, NULL, 1, 1, 1, 1, 'EQ', 'input', '',
        6, 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (79, 5, 'call_result', '调用结果', 'varchar(1000)', 'String', 'callResult', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'textarea', '', 7, 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (80, 5, 'call_responsetime', '响应时间', 'datetime', 'Date', 'callResponsetime', 0, 0, NULL, 1, 1, 1, 1, 'EQ',
        'datetime', '', 8, 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');
INSERT INTO "gen_table_column"
VALUES (81, 5, 'providercode', '提供方编码：dsjf,pdt', 'varchar(255)', 'String', 'providercode', 0, 0, NULL, 1, 1, 1, 1,
        'EQ', 'input', '', 9, 'admin', '2020-06-30 20:04:58', '', '2020-06-30 20:05:43');

-- ----------------------------
-- Table structure for qrtz_blob_triggers
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_blob_triggers";
CREATE TABLE "qrtz_blob_triggers"
(
    "sched_name"    text(120) NOT NULL,
    "trigger_name"  text(200) NOT NULL,
    "trigger_group" text(200) NOT NULL,
    "blob_data"     blob,
    PRIMARY KEY ("sched_name", "trigger_name", "trigger_group"),
    CONSTRAINT "qrtz_blob_triggers_ibfk_1" FOREIGN KEY ("sched_name", "trigger_name", "trigger_group") REFERENCES "qrtz_triggers" ("sched_name", "trigger_name", "trigger_group") ON DELETE RESTRICT ON UPDATE RESTRICT
);

-- ----------------------------
-- Table structure for qrtz_calendars
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_calendars";
CREATE TABLE "qrtz_calendars"
(
    "sched_name"    text(120) NOT NULL,
    "calendar_name" text(200) NOT NULL,
    "calendar"      blob NOT NULL,
    PRIMARY KEY ("sched_name", "calendar_name")
);

-- ----------------------------
-- Table structure for qrtz_cron_triggers
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_cron_triggers";
CREATE TABLE "qrtz_cron_triggers"
(
    "sched_name"      text(120) NOT NULL,
    "trigger_name"    text(200) NOT NULL,
    "trigger_group"   text(200) NOT NULL,
    "cron_expression" text(200) NOT NULL,
    "time_zone_id"    text(80),
    PRIMARY KEY ("sched_name", "trigger_name", "trigger_group"),
    CONSTRAINT "qrtz_cron_triggers_ibfk_1" FOREIGN KEY ("sched_name", "trigger_name", "trigger_group") REFERENCES "qrtz_triggers" ("sched_name", "trigger_name", "trigger_group") ON DELETE RESTRICT ON UPDATE RESTRICT
);

-- ----------------------------
-- Records of qrtz_cron_triggers
-- ----------------------------
INSERT INTO "qrtz_cron_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME1', 'DEFAULT', '0/10 * * * * ?', 'Asia/Shanghai');
INSERT INTO "qrtz_cron_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME100', 'SYSTEM', '0 */10 * * * ?', 'Asia/Shanghai');
INSERT INTO "qrtz_cron_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME101', 'SYSTEM', '*/10 * * * * ?', 'Asia/Shanghai');
INSERT INTO "qrtz_cron_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME2', 'DEFAULT', '0/15 * * * * ?', 'Asia/Shanghai');
INSERT INTO "qrtz_cron_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME3', 'DEFAULT', '0/20 * * * * ?', 'Asia/Shanghai');

-- ----------------------------
-- Table structure for qrtz_fired_triggers
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_fired_triggers";
CREATE TABLE "qrtz_fired_triggers"
(
    "sched_name"        text(120) NOT NULL,
    "entry_id"          text(95) NOT NULL,
    "trigger_name"      text(200) NOT NULL,
    "trigger_group"     text(200) NOT NULL,
    "instance_name"     text(200) NOT NULL,
    "fired_time"        integer(20) NOT NULL,
    "sched_time"        integer(20) NOT NULL,
    "priority"          integer(11) NOT NULL,
    "state"             text(16) NOT NULL,
    "job_name"          text(200),
    "job_group"         text(200),
    "is_nonconcurrent"  text(1),
    "requests_recovery" text(1),
    PRIMARY KEY ("sched_name", "entry_id")
);

-- ----------------------------
-- Records of qrtz_fired_triggers
-- ----------------------------
INSERT INTO "qrtz_fired_triggers"
VALUES ('RuoyiScheduler', 'DESKTOP-6MISMAG15960090974801596009097465', 'TASK_CLASS_NAME101', 'SYSTEM',
        'DESKTOP-6MISMAG1596009097480', 1596009450050, 1596009460000, 5, 'ACQUIRED', NULL, NULL, 0, 0);

-- ----------------------------
-- Table structure for qrtz_job_details
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_job_details";
CREATE TABLE "qrtz_job_details"
(
    "sched_name"        text(120) NOT NULL,
    "job_name"          text(200) NOT NULL,
    "job_group"         text(200) NOT NULL,
    "description"       text(250),
    "job_class_name"    text(250) NOT NULL,
    "is_durable"        text(1) NOT NULL,
    "is_nonconcurrent"  text(1) NOT NULL,
    "is_update_data"    text(1) NOT NULL,
    "requests_recovery" text(1) NOT NULL,
    "job_data"          blob,
    PRIMARY KEY ("sched_name", "job_name", "job_group")
);

-- ----------------------------
-- Records of qrtz_job_details
-- ----------------------------
INSERT INTO "qrtz_job_details"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME1', 'DEFAULT', NULL,
        'com.ruoyi.common.utils.job.QuartzDisallowConcurrentExecution', 0, 1, 0, 0,
        X'ACED0005737200156F72672E71756172747A2E4A6F62446174614D61709FB083E8BFA9B0CB020000787200266F72672E71756172747A2E7574696C732E537472696E674B65794469727479466C61674D61708208E8C3FBC55D280200015A0013616C6C6F77735472616E7369656E74446174617872001D6F72672E71756172747A2E7574696C732E4469727479466C61674D617013E62EAD28760ACE0200025A000564697274794C00036D617074000F4C6A6176612F7574696C2F4D61703B787001737200116A6176612E7574696C2E486173684D61700507DAC1C31660D103000246000A6C6F6164466163746F724900097468726573686F6C6478703F4000000000000C7708000000100000000174000F5441534B5F50524F5045525449455373720027636F6D2E72756F79692E70726F6A6563742E6D6F6E69746F722E646F6D61696E2E5379734A6F6200000000000000010200084C000A636F6E63757272656E747400124C6A6176612F6C616E672F537472696E673B4C000E63726F6E45787072657373696F6E71007E00094C000C696E766F6B6554617267657471007E00094C00086A6F6247726F757071007E00094C00056A6F6249647400104C6A6176612F6C616E672F4C6F6E673B4C00076A6F624E616D6571007E00094C000D6D697366697265506F6C69637971007E00094C000673746174757371007E000978720029636F6D2E72756F79692E6672616D65776F726B2E7765622E646F6D61696E2E42617365456E7469747900000000000000010200094C0009626567696E54696D6571007E00094C0008637265617465427971007E00094C000A63726561746554696D657400104C6A6176612F7574696C2F446174653B4C0007656E6454696D6571007E00094C0006706172616D7371007E00034C000672656D61726B71007E00094C000B73656172636856616C756571007E00094C0008757064617465427971007E00094C000A75706461746554696D6571007E000C78707074000561646D696E7372000E6A6176612E7574696C2E44617465686A81014B59741903000078707708000001622CDE29E07870707400007070707400013174000E302F3130202A202A202A202A203F74001172795461736B2E72794E6F506172616D7374000744454641554C547372000E6A6176612E6C616E672E4C6F6E673B8BE490CC8F23DF0200014A000576616C7565787200106A6176612E6C616E672E4E756D62657286AC951D0B94E08B02000078700000000000000001740018E7B3BBE7BB9FE9BB98E8AEA4EFBC88E697A0E58F82EFBC8974000133740001317800');
INSERT INTO "qrtz_job_details"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME100', 'SYSTEM', NULL, 'com.ruoyi.common.utils.job.QuartzJobExecution', 0, 0,
        0, 0,
        X'ACED0005737200156F72672E71756172747A2E4A6F62446174614D61709FB083E8BFA9B0CB020000787200266F72672E71756172747A2E7574696C732E537472696E674B65794469727479466C61674D61708208E8C3FBC55D280200015A0013616C6C6F77735472616E7369656E74446174617872001D6F72672E71756172747A2E7574696C732E4469727479466C61674D617013E62EAD28760ACE0200025A000564697274794C00036D617074000F4C6A6176612F7574696C2F4D61703B787001737200116A6176612E7574696C2E486173684D61700507DAC1C31660D103000246000A6C6F6164466163746F724900097468726573686F6C6478703F4000000000000C7708000000100000000174000F5441534B5F50524F5045525449455373720027636F6D2E72756F79692E70726F6A6563742E6D6F6E69746F722E646F6D61696E2E5379734A6F6200000000000000010200084C000A636F6E63757272656E747400124C6A6176612F6C616E672F537472696E673B4C000E63726F6E45787072657373696F6E71007E00094C000C696E766F6B6554617267657471007E00094C00086A6F6247726F757071007E00094C00056A6F6249647400104C6A6176612F6C616E672F4C6F6E673B4C00076A6F624E616D6571007E00094C000D6D697366697265506F6C69637971007E00094C000673746174757371007E000978720029636F6D2E72756F79692E6672616D65776F726B2E7765622E646F6D61696E2E42617365456E7469747900000000000000010200094C0009626567696E54696D6571007E00094C0008637265617465427971007E00094C000A63726561746554696D657400104C6A6176612F7574696C2F446174653B4C0007656E6454696D6571007E00094C0006706172616D7371007E00034C000672656D61726B71007E00094C000B73656172636856616C756571007E00094C0008757064617465427971007E00094C000A75706461746554696D6571007E000C7870707400007372000E6A6176612E7574696C2E44617465686A81014B59741903000078707708000001730EA64AF07870707400007070707400013074000E30202A2F3130202A202A202A203F74001764715461736B2E636865636B506474466C6F774461746174000653595354454D7372000E6A6176612E6C616E672E4C6F6E673B8BE490CC8F23DF0200014A000576616C7565787200106A6176612E6C616E672E4E756D62657286AC951D0B94E08B02000078700000000000000064740015E6B4BEE5A4A7E6B798E6B581E9878FE4BBBBE58AA174000131740001307800');
INSERT INTO "qrtz_job_details"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME101', 'SYSTEM', NULL,
        'com.ruoyi.common.utils.job.QuartzDisallowConcurrentExecution', 0, 1, 0, 0,
        X'ACED0005737200156F72672E71756172747A2E4A6F62446174614D61709FB083E8BFA9B0CB020000787200266F72672E71756172747A2E7574696C732E537472696E674B65794469727479466C61674D61708208E8C3FBC55D280200015A0013616C6C6F77735472616E7369656E74446174617872001D6F72672E71756172747A2E7574696C732E4469727479466C61674D617013E62EAD28760ACE0200025A000564697274794C00036D617074000F4C6A6176612F7574696C2F4D61703B787001737200116A6176612E7574696C2E486173684D61700507DAC1C31660D103000246000A6C6F6164466163746F724900097468726573686F6C6478703F4000000000000C7708000000100000000174000F5441534B5F50524F5045525449455373720027636F6D2E72756F79692E70726F6A6563742E6D6F6E69746F722E646F6D61696E2E5379734A6F6200000000000000010200084C000A636F6E63757272656E747400124C6A6176612F6C616E672F537472696E673B4C000E63726F6E45787072657373696F6E71007E00094C000C696E766F6B6554617267657471007E00094C00086A6F6247726F757071007E00094C00056A6F6249647400104C6A6176612F6C616E672F4C6F6E673B4C00076A6F624E616D6571007E00094C000D6D697366697265506F6C69637971007E00094C000673746174757371007E000978720029636F6D2E72756F79692E6672616D65776F726B2E7765622E646F6D61696E2E42617365456E7469747900000000000000010200094C0009626567696E54696D6571007E00094C0008637265617465427971007E00094C000A63726561746554696D657400104C6A6176612F7574696C2F446174653B4C0007656E6454696D6571007E00094C0006706172616D7371007E00034C000672656D61726B71007E00094C000B73656172636856616C756571007E00094C0008757064617465427971007E00094C000A75706461746554696D6571007E000C7870707400007372000E6A6176612E7574696C2E44617465686A81014B59741903000078707708000001736B9429F87870707400007070707400013174000E2A2F3130202A202A202A202A203F74001264715461736B2E636865636B59684461746174000653595354454D7372000E6A6176612E6C616E672E4C6F6E673B8BE490CC8F23DF0200014A000576616C7565787200106A6176612E6C616E672E4E756D62657286AC951D0B94E08B02000078700000000000000065740012E88EB7E58F96E9AA8CE58FB7E7BB93E69E9C74000131740001307800');
INSERT INTO "qrtz_job_details"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME2', 'DEFAULT', NULL,
        'com.ruoyi.common.utils.job.QuartzDisallowConcurrentExecution', 0, 1, 0, 0,
        X'ACED0005737200156F72672E71756172747A2E4A6F62446174614D61709FB083E8BFA9B0CB020000787200266F72672E71756172747A2E7574696C732E537472696E674B65794469727479466C61674D61708208E8C3FBC55D280200015A0013616C6C6F77735472616E7369656E74446174617872001D6F72672E71756172747A2E7574696C732E4469727479466C61674D617013E62EAD28760ACE0200025A000564697274794C00036D617074000F4C6A6176612F7574696C2F4D61703B787001737200116A6176612E7574696C2E486173684D61700507DAC1C31660D103000246000A6C6F6164466163746F724900097468726573686F6C6478703F4000000000000C7708000000100000000174000F5441534B5F50524F5045525449455373720027636F6D2E72756F79692E70726F6A6563742E6D6F6E69746F722E646F6D61696E2E5379734A6F6200000000000000010200084C000A636F6E63757272656E747400124C6A6176612F6C616E672F537472696E673B4C000E63726F6E45787072657373696F6E71007E00094C000C696E766F6B6554617267657471007E00094C00086A6F6247726F757071007E00094C00056A6F6249647400104C6A6176612F6C616E672F4C6F6E673B4C00076A6F624E616D6571007E00094C000D6D697366697265506F6C69637971007E00094C000673746174757371007E000978720029636F6D2E72756F79692E6672616D65776F726B2E7765622E646F6D61696E2E42617365456E7469747900000000000000010200094C0009626567696E54696D6571007E00094C0008637265617465427971007E00094C000A63726561746554696D657400104C6A6176612F7574696C2F446174653B4C0007656E6454696D6571007E00094C0006706172616D7371007E00034C000672656D61726B71007E00094C000B73656172636856616C756571007E00094C0008757064617465427971007E00094C000A75706461746554696D6571007E000C78707074000561646D696E7372000E6A6176612E7574696C2E44617465686A81014B59741903000078707708000001622CDE29E07870707400007070707400013174000E302F3135202A202A202A202A203F74001572795461736B2E7279506172616D7328277279272974000744454641554C547372000E6A6176612E6C616E672E4C6F6E673B8BE490CC8F23DF0200014A000576616C7565787200106A6176612E6C616E672E4E756D62657286AC951D0B94E08B02000078700000000000000002740018E7B3BBE7BB9FE9BB98E8AEA4EFBC88E69C89E58F82EFBC8974000133740001317800');
INSERT INTO "qrtz_job_details"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME3', 'DEFAULT', NULL,
        'com.ruoyi.common.utils.job.QuartzDisallowConcurrentExecution', 0, 1, 0, 0,
        X'ACED0005737200156F72672E71756172747A2E4A6F62446174614D61709FB083E8BFA9B0CB020000787200266F72672E71756172747A2E7574696C732E537472696E674B65794469727479466C61674D61708208E8C3FBC55D280200015A0013616C6C6F77735472616E7369656E74446174617872001D6F72672E71756172747A2E7574696C732E4469727479466C61674D617013E62EAD28760ACE0200025A000564697274794C00036D617074000F4C6A6176612F7574696C2F4D61703B787001737200116A6176612E7574696C2E486173684D61700507DAC1C31660D103000246000A6C6F6164466163746F724900097468726573686F6C6478703F4000000000000C7708000000100000000174000F5441534B5F50524F5045525449455373720027636F6D2E72756F79692E70726F6A6563742E6D6F6E69746F722E646F6D61696E2E5379734A6F6200000000000000010200084C000A636F6E63757272656E747400124C6A6176612F6C616E672F537472696E673B4C000E63726F6E45787072657373696F6E71007E00094C000C696E766F6B6554617267657471007E00094C00086A6F6247726F757071007E00094C00056A6F6249647400104C6A6176612F6C616E672F4C6F6E673B4C00076A6F624E616D6571007E00094C000D6D697366697265506F6C69637971007E00094C000673746174757371007E000978720029636F6D2E72756F79692E6672616D65776F726B2E7765622E646F6D61696E2E42617365456E7469747900000000000000010200094C0009626567696E54696D6571007E00094C0008637265617465427971007E00094C000A63726561746554696D657400104C6A6176612F7574696C2F446174653B4C0007656E6454696D6571007E00094C0006706172616D7371007E00034C000672656D61726B71007E00094C000B73656172636856616C756571007E00094C0008757064617465427971007E00094C000A75706461746554696D6571007E000C78707074000561646D696E7372000E6A6176612E7574696C2E44617465686A81014B59741903000078707708000001622CDE29E07870707400007070707400013174000E302F3230202A202A202A202A203F74003872795461736B2E72794D756C7469706C65506172616D7328277279272C20747275652C20323030304C2C203331362E3530442C203130302974000744454641554C547372000E6A6176612E6C616E672E4C6F6E673B8BE490CC8F23DF0200014A000576616C7565787200106A6176612E6C616E672E4E756D62657286AC951D0B94E08B02000078700000000000000003740018E7B3BBE7BB9FE9BB98E8AEA4EFBC88E5A49AE58F82EFBC8974000133740001317800');

-- ----------------------------
-- Table structure for qrtz_locks
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_locks";
CREATE TABLE "qrtz_locks"
(
    "sched_name" text(120) NOT NULL,
    "lock_name"  text(40) NOT NULL,
    PRIMARY KEY ("sched_name", "lock_name")
);

-- ----------------------------
-- Records of qrtz_locks
-- ----------------------------
INSERT INTO "qrtz_locks"
VALUES ('RuoyiScheduler', 'STATE_ACCESS');
INSERT INTO "qrtz_locks"
VALUES ('RuoyiScheduler', 'TRIGGER_ACCESS');

-- ----------------------------
-- Table structure for qrtz_paused_trigger_grps
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_paused_trigger_grps";
CREATE TABLE "qrtz_paused_trigger_grps"
(
    "sched_name"    text(120) NOT NULL,
    "trigger_group" text(200) NOT NULL,
    PRIMARY KEY ("sched_name", "trigger_group")
);

-- ----------------------------
-- Table structure for qrtz_scheduler_state
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_scheduler_state";
CREATE TABLE "qrtz_scheduler_state"
(
    "sched_name"        text(120) NOT NULL,
    "instance_name"     text(200) NOT NULL,
    "last_checkin_time" integer(20) NOT NULL,
    "checkin_interval"  integer(20) NOT NULL,
    PRIMARY KEY ("sched_name", "instance_name")
);

-- ----------------------------
-- Records of qrtz_scheduler_state
-- ----------------------------
INSERT INTO "qrtz_scheduler_state"
VALUES ('RuoyiScheduler', 'DESKTOP-6MISMAG1596009097480', 1596009449214, 15000);

-- ----------------------------
-- Table structure for qrtz_simple_triggers
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_simple_triggers";
CREATE TABLE "qrtz_simple_triggers"
(
    "sched_name"      text(120) NOT NULL,
    "trigger_name"    text(200) NOT NULL,
    "trigger_group"   text(200) NOT NULL,
    "repeat_count"    integer(20) NOT NULL,
    "repeat_interval" integer(20) NOT NULL,
    "times_triggered" integer(20) NOT NULL,
    PRIMARY KEY ("sched_name", "trigger_name", "trigger_group"),
    CONSTRAINT "qrtz_simple_triggers_ibfk_1" FOREIGN KEY ("sched_name", "trigger_name", "trigger_group") REFERENCES "qrtz_triggers" ("sched_name", "trigger_name", "trigger_group") ON DELETE RESTRICT ON UPDATE RESTRICT
);

-- ----------------------------
-- Table structure for qrtz_simprop_triggers
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_simprop_triggers";
CREATE TABLE "qrtz_simprop_triggers"
(
    "sched_name"    text(120) NOT NULL,
    "trigger_name"  text(200) NOT NULL,
    "trigger_group" text(200) NOT NULL,
    "str_prop_1"    text(512),
    "str_prop_2"    text(512),
    "str_prop_3"    text(512),
    "int_prop_1"    integer(11),
    "int_prop_2"    integer(11),
    "long_prop_1"   integer(20),
    "long_prop_2"   integer(20),
    "dec_prop_1"    real(13,4),
    "dec_prop_2"    real(13,4),
    "bool_prop_1"   text(1),
    "bool_prop_2"   text(1),
    PRIMARY KEY ("sched_name", "trigger_name", "trigger_group"),
    CONSTRAINT "qrtz_simprop_triggers_ibfk_1" FOREIGN KEY ("sched_name", "trigger_name", "trigger_group") REFERENCES "qrtz_triggers" ("sched_name", "trigger_name", "trigger_group") ON DELETE RESTRICT ON UPDATE RESTRICT
);

-- ----------------------------
-- Table structure for qrtz_triggers
-- ----------------------------
DROP TABLE IF EXISTS "qrtz_triggers";
CREATE TABLE "qrtz_triggers"
(
    "sched_name"     text(120) NOT NULL,
    "trigger_name"   text(200) NOT NULL,
    "trigger_group"  text(200) NOT NULL,
    "job_name"       text(200) NOT NULL,
    "job_group"      text(200) NOT NULL,
    "description"    text(250),
    "next_fire_time" integer(20),
    "prev_fire_time" integer(20),
    "priority"       integer(11),
    "trigger_state"  text(16) NOT NULL,
    "trigger_type"   text(8) NOT NULL,
    "start_time"     integer(20) NOT NULL,
    "end_time"       integer(20),
    "calendar_name"  text(200),
    "misfire_instr"  integer(6),
    "job_data"       blob,
    PRIMARY KEY ("sched_name", "trigger_name", "trigger_group"),
    CONSTRAINT "qrtz_triggers_ibfk_1" FOREIGN KEY ("sched_name", "job_name", "job_group") REFERENCES "qrtz_job_details" ("sched_name", "job_name", "job_group") ON DELETE RESTRICT ON UPDATE RESTRICT
);

-- ----------------------------
-- Records of qrtz_triggers
-- ----------------------------
INSERT INTO "qrtz_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME1', 'DEFAULT', 'TASK_CLASS_NAME1', 'DEFAULT', NULL, 1596009100000, -1, 5,
        'PAUSED', 'CRON', 1596009097000, 0, NULL, 2, '');
INSERT INTO "qrtz_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME100', 'SYSTEM', 'TASK_CLASS_NAME100', 'SYSTEM', NULL, 1596009600000, -1, 5,
        'WAITING', 'CRON', 1596009098000, 0, NULL, -1, '');
INSERT INTO "qrtz_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME101', 'SYSTEM', 'TASK_CLASS_NAME101', 'SYSTEM', NULL, 1596009460000,
        1596009450000, 5, 'ACQUIRED', 'CRON', 1596009098000, 0, NULL, -1, '');
INSERT INTO "qrtz_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME2', 'DEFAULT', 'TASK_CLASS_NAME2', 'DEFAULT', NULL, 1596009105000, -1, 5,
        'PAUSED', 'CRON', 1596009098000, 0, NULL, 2, '');
INSERT INTO "qrtz_triggers"
VALUES ('RuoyiScheduler', 'TASK_CLASS_NAME3', 'DEFAULT', 'TASK_CLASS_NAME3', 'DEFAULT', NULL, 1596009100000, -1, 5,
        'PAUSED', 'CRON', 1596009098000, 0, NULL, 2, '');

-- ----------------------------
-- Table structure for sqlite_sequence
-- ----------------------------
DROP TABLE IF EXISTS "sqlite_sequence";
CREATE TABLE "sqlite_sequence"
(
    "name",
    "seq"
);

-- ----------------------------
-- Records of sqlite_sequence
-- ----------------------------
INSERT INTO "sqlite_sequence"
VALUES ('dq_app_user', 72);
INSERT INTO "sqlite_sequence"
VALUES ('gen_table', 5);
INSERT INTO "sqlite_sequence"
VALUES ('gen_table_column', 81);
INSERT INTO "sqlite_sequence"
VALUES ('sys_config', 112);
INSERT INTO "sqlite_sequence"
VALUES ('sys_dept', 109);
INSERT INTO "sqlite_sequence"
VALUES ('sys_dict_data', 137);
INSERT INTO "sqlite_sequence"
VALUES ('sys_dict_type', 109);
INSERT INTO "sqlite_sequence"
VALUES ('sys_job_log', 83834);
INSERT INTO "sqlite_sequence"
VALUES ('sys_logininfor', 544);
INSERT INTO "sqlite_sequence"
VALUES ('sys_menu', 2015);
INSERT INTO "sqlite_sequence"
VALUES ('sys_notice', 3);
INSERT INTO "sqlite_sequence"
VALUES ('sys_oper_log', 326);
INSERT INTO "sqlite_sequence"
VALUES ('sys_post', 4);
INSERT INTO "sqlite_sequence"
VALUES ('sys_role', 2);
INSERT INTO "sqlite_sequence"
VALUES ('sys_user', 2);

-- ----------------------------
-- Table structure for sys_config
-- ----------------------------
DROP TABLE IF EXISTS "sys_config";
CREATE TABLE "sys_config"
(
    "config_id"    integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "config_name"  text(100),
    "config_key"   text(100),
    "config_value" text(500),
    "config_type"  text(1),
    "create_by"    text(64),
    "create_time"  text,
    "update_by"    text(64),
    "update_time"  text,
    "remark"       text(500)
);

-- ----------------------------
-- Records of sys_config
-- ----------------------------
INSERT INTO "sys_config"
VALUES (1, '主框架页-默认皮肤样式名称', 'sys.index.skinName', 'skin-blue', 'Y', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '蓝色 skin-blue、绿色 skin-green、紫色 skin-purple、红色 skin-red、黄色 skin-yellow');
INSERT INTO "sys_config"
VALUES (2, '用户管理-账号初始密码', 'sys.user.initPassword', 123456, 'Y', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '初始化密码 123456');
INSERT INTO "sys_config"
VALUES (3, '主框架页-侧边栏主题', 'sys.index.sideTheme', 'theme-dark', 'Y', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '深色主题theme-dark，浅色主题theme-light');
INSERT INTO "sys_config"
VALUES (100, '验号单价', 'single_bid_yh_price', 3, 'N', 'admin', '2020-07-21 10:34:46', 'admin', '2020-07-21 14:15:47',
        '金币');
INSERT INTO "sys_config"
VALUES (101, '验号成本', 'single_cost_yh_price', 0.03, 'N', 'admin', '2020-07-21 10:35:09', 'admin',
        '2020-07-21 14:15:51', '元');
INSERT INTO "sys_config"
VALUES (105, '打标授权', 'dsjy_token', '4956de596a564ee3be14604d0a0e4e9e', 'N', 'admin', '2020-07-21 10:55:20', 'admin',
        '2020-07-21 14:15:55', NULL);
INSERT INTO "sys_config"
VALUES (107, '打标单价', 'single_bid_marking_price', 3, 'N', 'admin', '2020-07-21 14:07:45', 'admin',
        '2020-07-21 14:15:59', '金币');
INSERT INTO "sys_config"
VALUES (108, '打标成本', 'single_cost_marking_price', 0.06, 'N', 'admin', '2020-07-21 14:08:45', 'admin',
        '2020-07-21 14:16:02', '元');
INSERT INTO "sys_config"
VALUES (109, '派大淘流量单价', 'single_bid_pdtflow_price', 3, 'N', 'admin', '2020-07-21 14:11:04', 'admin',
        '2020-07-21 14:16:10', '金币');
INSERT INTO "sys_config"
VALUES (110, '派大淘流量成本', 'single_cost_pdtflow_price', 0.04, 'N', 'admin', '2020-07-21 14:12:05', 'admin',
        '2020-07-21 14:16:14', '元');
INSERT INTO "sys_config"
VALUES (111, '派大淘APPID', 'pdt_appid', 11217778, 'N', 'admin', '2020-07-21 14:14:47', 'admin', '2020-07-21 14:16:19',
        NULL);
INSERT INTO "sys_config"
VALUES (112, '派大淘密钥', 'pdt_appsecrect', '02479ea0b17e9cfbe5513dd8f9503dd4', 'N', 'admin', '2020-07-21 14:15:23',
        'admin', '2020-07-21 14:16:22', NULL);

-- ----------------------------
-- Table structure for sys_dept
-- ----------------------------
DROP TABLE IF EXISTS "sys_dept";
CREATE TABLE "sys_dept"
(
    "dept_id"     integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "parent_id"   integer(20),
    "ancestors"   text(50),
    "dept_name"   text(30),
    "order_num"   integer(11),
    "leader"      text(20),
    "phone"       text(11),
    "email"       text(50),
    "status"      text(1),
    "del_flag"    text(1),
    "create_by"   text(64),
    "create_time" text,
    "update_by"   text(64),
    "update_time" text
);

-- ----------------------------
-- Records of sys_dept
-- ----------------------------
INSERT INTO "sys_dept"
VALUES (100, 0, 0, '若依科技', 0, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (101, 100, '0,100', '深圳总公司', 1, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (102, 100, '0,100', '长沙分公司', 2, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (103, 101, '0,100,101', '研发部门', 1, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (104, 101, '0,100,101', '市场部门', 2, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (105, 101, '0,100,101', '测试部门', 3, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (106, 101, '0,100,101', '财务部门', 4, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (107, 101, '0,100,101', '运维部门', 5, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (108, 102, '0,100,102', '市场部门', 1, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');
INSERT INTO "sys_dept"
VALUES (109, 102, '0,100,102', '财务部门', 2, '若依', 15888888888, 'ry@qq.com', 0, 0, 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00');

-- ----------------------------
-- Table structure for sys_dict_data
-- ----------------------------
DROP TABLE IF EXISTS "sys_dict_data";
CREATE TABLE "sys_dict_data"
(
    "dict_code"   integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "dict_sort"   integer(11),
    "dict_label"  text(100),
    "dict_value"  text(100),
    "dict_type"   text(100),
    "css_class"   text(100),
    "list_class"  text(100),
    "is_default"  text(1),
    "status"      text(1),
    "create_by"   text(64),
    "create_time" text,
    "update_by"   text(64),
    "update_time" text,
    "remark"      text(500)
);

-- ----------------------------
-- Records of sys_dict_data
-- ----------------------------
INSERT INTO "sys_dict_data"
VALUES (1, 1, '男', 0, 'sys_user_sex', '', '', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '性别男');
INSERT INTO "sys_dict_data"
VALUES (2, 2, '女', 1, 'sys_user_sex', '', '', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '性别女');
INSERT INTO "sys_dict_data"
VALUES (3, 3, '未知', 2, 'sys_user_sex', '', '', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '性别未知');
INSERT INTO "sys_dict_data"
VALUES (4, 1, '显示', 0, 'sys_show_hide', '', 'primary', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '显示菜单');
INSERT INTO "sys_dict_data"
VALUES (5, 2, '隐藏', 1, 'sys_show_hide', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '隐藏菜单');
INSERT INTO "sys_dict_data"
VALUES (6, 1, '正常', 0, 'sys_normal_disable', '', 'primary', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '正常状态');
INSERT INTO "sys_dict_data"
VALUES (7, 2, '停用', 1, 'sys_normal_disable', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '停用状态');
INSERT INTO "sys_dict_data"
VALUES (8, 1, '正常', 0, 'sys_job_status', '', 'primary', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '正常状态');
INSERT INTO "sys_dict_data"
VALUES (9, 2, '暂停', 1, 'sys_job_status', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '停用状态');
INSERT INTO "sys_dict_data"
VALUES (10, 1, '默认', 'DEFAULT', 'sys_job_group', '', '', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '默认分组');
INSERT INTO "sys_dict_data"
VALUES (11, 2, '系统', 'SYSTEM', 'sys_job_group', '', '', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '系统分组');
INSERT INTO "sys_dict_data"
VALUES (12, 1, '是', 'Y', 'sys_yes_no', '', 'primary', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '系统默认是');
INSERT INTO "sys_dict_data"
VALUES (13, 2, '否', 'N', 'sys_yes_no', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '系统默认否');
INSERT INTO "sys_dict_data"
VALUES (14, 1, '通知', 1, 'sys_notice_type', '', 'warning', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '通知');
INSERT INTO "sys_dict_data"
VALUES (15, 2, '公告', 2, 'sys_notice_type', '', 'success', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '公告');
INSERT INTO "sys_dict_data"
VALUES (16, 1, '正常', 0, 'sys_notice_status', '', 'primary', 'Y', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '正常状态');
INSERT INTO "sys_dict_data"
VALUES (17, 2, '关闭', 1, 'sys_notice_status', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '关闭状态');
INSERT INTO "sys_dict_data"
VALUES (18, 1, '新增', 1, 'sys_oper_type', '', 'info', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '新增操作');
INSERT INTO "sys_dict_data"
VALUES (19, 2, '修改', 2, 'sys_oper_type', '', 'info', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '修改操作');
INSERT INTO "sys_dict_data"
VALUES (20, 3, '删除', 3, 'sys_oper_type', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '删除操作');
INSERT INTO "sys_dict_data"
VALUES (21, 4, '授权', 4, 'sys_oper_type', '', 'primary', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '授权操作');
INSERT INTO "sys_dict_data"
VALUES (22, 5, '导出', 5, 'sys_oper_type', '', 'warning', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '导出操作');
INSERT INTO "sys_dict_data"
VALUES (23, 6, '导入', 6, 'sys_oper_type', '', 'warning', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '导入操作');
INSERT INTO "sys_dict_data"
VALUES (24, 7, '强退', 7, 'sys_oper_type', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '强退操作');
INSERT INTO "sys_dict_data"
VALUES (25, 8, '生成代码', 8, 'sys_oper_type', '', 'warning', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '生成操作');
INSERT INTO "sys_dict_data"
VALUES (26, 9, '清空数据', 9, 'sys_oper_type', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '清空操作');
INSERT INTO "sys_dict_data"
VALUES (27, 1, '成功', 0, 'sys_common_status', '', 'primary', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '正常状态');
INSERT INTO "sys_dict_data"
VALUES (28, 2, '失败', 1, 'sys_common_status', '', 'danger', 'N', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '停用状态');
INSERT INTO "sys_dict_data"
VALUES (101, 0, '审核通过', 1, 'dq_app_user_account_status', NULL, NULL, 'N', 0, 'admin', '2020-06-20 10:17:46', '',
        NULL, NULL);
INSERT INTO "sys_dict_data"
VALUES (103, 0, '停用', 9, 'dq_app_user_account_status', NULL, NULL, 'N', 0, 'admin', '2020-06-20 10:18:09', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (104, 0, '普通会员', 0, 'dq_app_user_role_id', NULL, NULL, 'N', 0, 'admin', '2020-06-20 10:19:50', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (105, 0, '系统会员', 1, 'dq_app_user_role_id', NULL, NULL, 'N', 0, 'admin', '2020-06-20 10:20:00', 'admin',
        '2020-06-20 10:20:08', NULL);
INSERT INTO "sys_dict_data"
VALUES (106, 0, '手动充值', 'online', 'dq_pay_order_pay_platform', NULL, NULL, 'N', 0, 'admin', '2020-06-20 15:46:36',
        '', NULL, NULL);
INSERT INTO "sys_dict_data"
VALUES (107, 1, '支付宝', 'alipay', 'dq_pay_order_pay_platform', NULL, NULL, 'N', 0, 'admin', '2020-06-20 15:46:56', '',
        NULL, NULL);
INSERT INTO "sys_dict_data"
VALUES (108, 3, '微信充值', 'wxpay', 'dq_pay_order_pay_platform', NULL, NULL, 'N', 0, 'admin', '2020-06-20 15:47:12',
        '', NULL, NULL);
INSERT INTO "sys_dict_data"
VALUES (109, 0, '在线支付', 10, 'dq_pay_order_pay_type', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:02:41', '', NULL,
        '支付类型(10=在线支付、20=线下支付)');
INSERT INTO "sys_dict_data"
VALUES (110, 1, '线下支付', 20, 'dq_pay_order_pay_type', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:03:06', '', NULL,
        '支付类型(10=在线支付、20=线下支付)');
INSERT INTO "sys_dict_data"
VALUES (111, 1, '待支付', 10, 'dq_pay_order_pay_status', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:04:30', '', NULL,
        '支付状态(10=待支付、20=支付失败、30=支付成功、40=已关闭、50=已取消)');
INSERT INTO "sys_dict_data"
VALUES (112, 2, '支付失败', 20, 'dq_pay_order_pay_status', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:04:39', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (113, 3, '支付成功', 30, 'dq_pay_order_pay_status', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:04:49', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (114, 4, '已关闭', 40, 'dq_pay_order_pay_status', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:04:57', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (115, 5, '已取消', 50, 'dq_pay_order_pay_status', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:05:07', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (116, 0, '老的', 0, 'dq_pay_order_recharge_type', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:07:27', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (117, 0, '对冲', 1, 'dq_pay_order_recharge_type', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:07:33', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (118, 0, '漏充', 2, 'dq_pay_order_recharge_type', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:07:44', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (119, 0, '补偿亏损', 3, 'dq_pay_order_recharge_type', NULL, NULL, 'N', 0, 'admin', '2020-06-20 16:07:52', '',
        NULL, NULL);
INSERT INTO "sys_dict_data"
VALUES (120, 1, '精准流量', 48, 'dq_pdt_flow_task_type', NULL, NULL, 'N', 0, 'admin', '2020-06-23 17:57:27', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (121, 1, '流量任务', 1, 'dq_pdt_flow_task_type', NULL, NULL, 'N', 0, 'admin', '2020-06-23 17:58:45', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (122, 1, '收藏任务', 2, 'dq_pdt_flow_task_type', NULL, NULL, 'N', 0, 'admin', '2020-06-23 17:59:08', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (123, 1, '加购任务', 3, 'dq_pdt_flow_task_type', NULL, NULL, 'N', 0, 'admin', '2020-06-23 17:59:30', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (124, 1, '商品推荐', 106, 'dq_pdt_flow_task_type', NULL, NULL, 'N', 0, 'admin', '2020-06-23 17:59:55', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (125, 1, '关注店铺', 60, 'dq_pdt_flow_task_type', NULL, NULL, 'N', 0, 'admin', '2020-06-23 18:00:11', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (126, 1, '开团提醒', 101, 'dq_pdt_flow_task_type', NULL, NULL, 'N', 0, 'admin', '2020-06-23 18:00:26', '', NULL,
        NULL);
INSERT INTO "sys_dict_data"
VALUES (128, 1, '浏览121-180秒（免费）', 1, 'dq_browse_goods_time_id', NULL, NULL, 'N', 0, 'admin', '2020-06-23 19:45:34',
        'admin', '2020-06-23 19:51:31', 0);
INSERT INTO "sys_dict_data"
VALUES (129, 1, '浏览181-300秒（ 5金币）', 2, 'dq_browse_goods_time_id', NULL, NULL, 'N', 0, 'admin',
        '2020-06-23 19:46:32', 'admin', '2020-06-23 19:51:42', 5);
INSERT INTO "sys_dict_data"
VALUES (130, 1, '浏览301-420秒（ 10金币）', 3, 'dq_browse_goods_time_id', NULL, NULL, 'N', 0, 'admin',
        '2020-06-23 19:47:23', 'admin', '2020-06-23 19:51:35', 10);
INSERT INTO "sys_dict_data"
VALUES (131, 1, '深入1个商品（免费）', 1, 'dq_browse_other_goods', NULL, NULL, 'N', 0, 'admin', '2020-06-23 19:48:59',
        'admin', '2020-06-23 19:51:04', 0);
INSERT INTO "sys_dict_data"
VALUES (132, 1, '随机深入浏览1-2个商品（+2金币)', 2, 'dq_browse_other_goods', NULL, NULL, 'N', 0, 'admin',
        '2020-06-23 19:49:31', 'admin', '2020-06-23 19:50:57', '	2');
INSERT INTO "sys_dict_data"
VALUES (133, 1, '随机深入浏览1-3个商品（+3金币）', 3, 'dq_browse_other_goods', NULL, NULL, 'N', 0, 'admin',
        '2020-06-23 19:49:41', 'admin', '2020-06-23 19:50:54', 3);
INSERT INTO "sys_dict_data"
VALUES (134, 1, '深入浏览2个商品（+3金币）', 5, 'dq_browse_other_goods', NULL, NULL, 'N', 0, 'admin',
        '2020-06-23 19:49:59', 'admin', '2020-06-23 19:50:43', 3);
INSERT INTO "sys_dict_data"
VALUES (135, 1, '深入浏览3个商品（+4金币）', 6, 'dq_browse_other_goods', NULL, NULL, 'N', 0, 'admin',
        '2020-06-23 19:50:14', 'admin', '2020-06-23 19:50:37', 4);
INSERT INTO "sys_dict_data"
VALUES (136, 1, '不深入浏览', 7, 'dq_browse_other_goods', NULL, NULL, 'N', 0, 'admin', '2020-06-23 19:50:30', 'admin',
        '2020-06-23 19:50:40', 0);
INSERT INTO "sys_dict_data"
VALUES (137, 4, '平台奖励', 'award', 'dq_pay_order_pay_platform', NULL, NULL, 'N', 0, 'admin', '2020-07-10 17:25:38',
        'admin', '2020-07-10 17:25:47', NULL);

-- ----------------------------
-- Table structure for sys_dict_type
-- ----------------------------
DROP TABLE IF EXISTS "sys_dict_type";
CREATE TABLE "sys_dict_type"
(
    "dict_id"     integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "dict_name"   text(100),
    "dict_type"   text(100),
    "status"      text(1),
    "create_by"   text(64),
    "create_time" text,
    "update_by"   text(64),
    "update_time" text,
    "remark"      text(500)
);

-- ----------------------------
-- Records of sys_dict_type
-- ----------------------------
INSERT INTO "sys_dict_type"
VALUES (1, '用户性别', 'sys_user_sex', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '用户性别列表');
INSERT INTO "sys_dict_type"
VALUES (2, '菜单状态', 'sys_show_hide', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '菜单状态列表');
INSERT INTO "sys_dict_type"
VALUES (3, '系统开关', 'sys_normal_disable', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '系统开关列表');
INSERT INTO "sys_dict_type"
VALUES (4, '任务状态', 'sys_job_status', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '任务状态列表');
INSERT INTO "sys_dict_type"
VALUES (5, '任务分组', 'sys_job_group', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '任务分组列表');
INSERT INTO "sys_dict_type"
VALUES (6, '系统是否', 'sys_yes_no', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '系统是否列表');
INSERT INTO "sys_dict_type"
VALUES (7, '通知类型', 'sys_notice_type', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '通知类型列表');
INSERT INTO "sys_dict_type"
VALUES (8, '通知状态', 'sys_notice_status', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '通知状态列表');
INSERT INTO "sys_dict_type"
VALUES (9, '操作类型', 'sys_oper_type', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '操作类型列表');
INSERT INTO "sys_dict_type"
VALUES (10, '系统状态', 'sys_common_status', 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00',
        '登录状态列表');
INSERT INTO "sys_dict_type"
VALUES (100, '会员状态', 'dq_app_user_account_status', 0, 'admin', '2020-06-20 10:16:51', '', NULL, NULL);
INSERT INTO "sys_dict_type"
VALUES (101, '会员等级', 'dq_app_user_role_id', 0, 'admin', '2020-06-20 10:19:22', '', NULL, NULL);
INSERT INTO "sys_dict_type"
VALUES (102, '充值方式', 'dq_pay_order_pay_platform', 0, 'admin', '2020-06-20 15:46:06', '', NULL, NULL);
INSERT INTO "sys_dict_type"
VALUES (103, '支付类型', 'dq_pay_order_pay_type', 0, 'admin', '2020-06-20 16:01:41', '', NULL, NULL);
INSERT INTO "sys_dict_type"
VALUES (104, '支付状态', 'dq_pay_order_pay_status', 0, 'admin', '2020-06-20 16:03:57', '', NULL,
        '支付状态(10=待支付、20=支付失败、30=支付成功、40=已关闭、50=已取消)');
INSERT INTO "sys_dict_type"
VALUES (105, '充值类型', 'dq_pay_order_recharge_type', 0, 'admin', '2020-06-20 16:07:13', '', NULL, NULL);
INSERT INTO "sys_dict_type"
VALUES (106, '派大淘流量大类', 'dq_pdt_flow_task_type', 0, 'admin', '2020-06-23 17:54:24', '', NULL, NULL);
INSERT INTO "sys_dict_type"
VALUES (107, '浏览任务商品时间', 'dq_browse_goods_time_id', 0, 'admin', '2020-06-23 19:35:44', 'admin',
        '2020-06-23 19:41:06', '浏览任务商品时间id,不传默认为1,');
INSERT INTO "sys_dict_type"
VALUES (108, '深度浏览其他商品', 'dq_browse_other_goods', 0, 'admin', '2020-06-23 19:38:41', 'admin',
        '2020-06-23 19:41:24', '需要深度浏览其他商品时,不传默认为1,');
INSERT INTO "sys_dict_type"
VALUES (109, '派大淘流量子类', 'dq_pdt_flow_task_sub_type', 0, 'admin', '2020-06-23 20:17:25', '', NULL, NULL);

-- ----------------------------
-- Table structure for sys_job
-- ----------------------------
DROP TABLE IF EXISTS "sys_job";
CREATE TABLE "sys_job"
(
    "job_id"          integer NOT NULL,
    "job_name"        text(64) NOT NULL,
    "job_group"       text(64) NOT NULL,
    "invoke_target"   text(500) NOT NULL,
    "cron_expression" text(255),
    "misfire_policy"  text(20),
    "concurrent"      text(1),
    "status"          text(1),
    "create_by"       text(64),
    "create_time"     text,
    "update_by"       text(64),
    "update_time"     text,
    "remark"          text(500),
    PRIMARY KEY ("job_id", "job_name", "job_group")
);

-- ----------------------------
-- Table structure for sys_job_log
-- ----------------------------
DROP TABLE IF EXISTS "sys_job_log";
CREATE TABLE "sys_job_log"
(
    "job_log_id"     integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "job_name"       text(64) NOT NULL,
    "job_group"      text(64) NOT NULL,
    "invoke_target"  text(500) NOT NULL,
    "job_message"    text(500),
    "status"         text(1),
    "exception_info" text(2000),
    "create_time"    text
);

-- ----------------------------
-- Table structure for sys_logininfor
-- ----------------------------
DROP TABLE IF EXISTS "sys_logininfor";
CREATE TABLE "sys_logininfor"
(
    "info_id"        integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "user_name"      text(50),
    "ipaddr"         text(50),
    "login_location" text(255),
    "browser"        text(50),
    "os"             text(50),
    "status"         text(1),
    "msg"            text(255),
    "login_time"     text
);

-- ----------------------------
-- Records of sys_logininfor
-- ----------------------------
INSERT INTO "sys_logininfor"
VALUES (100, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-10 16:30:51');
INSERT INTO "sys_logininfor"
VALUES (101, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码错误', '2020-06-20 09:53:37');
INSERT INTO "sys_logininfor"
VALUES (102, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 09:53:44');
INSERT INTO "sys_logininfor"
VALUES (103, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-20 10:05:10');
INSERT INTO "sys_logininfor"
VALUES (104, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 10:05:17');
INSERT INTO "sys_logininfor"
VALUES (105, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 11:47:44');
INSERT INTO "sys_logininfor"
VALUES (106, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 15:07:09');
INSERT INTO "sys_logininfor"
VALUES (107, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-20 15:20:07');
INSERT INTO "sys_logininfor"
VALUES (108, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 15:20:14');
INSERT INTO "sys_logininfor"
VALUES (109, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-20 15:22:54');
INSERT INTO "sys_logininfor"
VALUES (110, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码错误', '2020-06-20 15:23:04');
INSERT INTO "sys_logininfor"
VALUES (111, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 15:23:11');
INSERT INTO "sys_logininfor"
VALUES (112, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-20 15:26:09');
INSERT INTO "sys_logininfor"
VALUES (113, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 15:26:16');
INSERT INTO "sys_logininfor"
VALUES (114, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 16:52:34');
INSERT INTO "sys_logininfor"
VALUES (115, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 17:28:47');
INSERT INTO "sys_logininfor"
VALUES (116, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-20 18:07:26');
INSERT INTO "sys_logininfor"
VALUES (117, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-20 18:07:36');
INSERT INTO "sys_logininfor"
VALUES (118, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 10:17:53');
INSERT INTO "sys_logininfor"
VALUES (119, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 10:32:14');
INSERT INTO "sys_logininfor"
VALUES (120, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 10:32:21');
INSERT INTO "sys_logininfor"
VALUES (121, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 10:33:28');
INSERT INTO "sys_logininfor"
VALUES (122, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 10:33:40');
INSERT INTO "sys_logininfor"
VALUES (123, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 10:34:17');
INSERT INTO "sys_logininfor"
VALUES (124, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 10:34:25');
INSERT INTO "sys_logininfor"
VALUES (125, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-21 11:17:48');
INSERT INTO "sys_logininfor"
VALUES (126, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 11:17:57');
INSERT INTO "sys_logininfor"
VALUES (127, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 12:01:45');
INSERT INTO "sys_logininfor"
VALUES (128, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 12:01:54');
INSERT INTO "sys_logininfor"
VALUES (129, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 14:59:36');
INSERT INTO "sys_logininfor"
VALUES (130, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 15:18:20');
INSERT INTO "sys_logininfor"
VALUES (131, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 15:18:28');
INSERT INTO "sys_logininfor"
VALUES (132, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 15:20:08');
INSERT INTO "sys_logininfor"
VALUES (133, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 15:20:17');
INSERT INTO "sys_logininfor"
VALUES (134, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 15:21:30');
INSERT INTO "sys_logininfor"
VALUES (135, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 15:21:41');
INSERT INTO "sys_logininfor"
VALUES (136, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 15:24:36');
INSERT INTO "sys_logininfor"
VALUES (137, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 15:24:44');
INSERT INTO "sys_logininfor"
VALUES (138, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-21 15:44:30');
INSERT INTO "sys_logininfor"
VALUES (139, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 15:44:38');
INSERT INTO "sys_logininfor"
VALUES (140, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 16:56:14');
INSERT INTO "sys_logininfor"
VALUES (141, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-21 17:10:32');
INSERT INTO "sys_logininfor"
VALUES (142, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:18:53');
INSERT INTO "sys_logininfor"
VALUES (143, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:19:51');
INSERT INTO "sys_logininfor"
VALUES (144, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-22 15:20:53');
INSERT INTO "sys_logininfor"
VALUES (145, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-22 15:21:55');
INSERT INTO "sys_logininfor"
VALUES (146, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:22:27');
INSERT INTO "sys_logininfor"
VALUES (147, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:23:24');
INSERT INTO "sys_logininfor"
VALUES (148, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:31:57');
INSERT INTO "sys_logininfor"
VALUES (149, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:40:02');
INSERT INTO "sys_logininfor"
VALUES (150, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:50:42');
INSERT INTO "sys_logininfor"
VALUES (151, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-22 15:53:59');
INSERT INTO "sys_logininfor"
VALUES (152, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-22 15:54:13');
INSERT INTO "sys_logininfor"
VALUES (153, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:54:52');
INSERT INTO "sys_logininfor"
VALUES (154, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 15:56:43');
INSERT INTO "sys_logininfor"
VALUES (155, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 16:17:49');
INSERT INTO "sys_logininfor"
VALUES (156, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, NULL, '2020-06-22 16:18:24');
INSERT INTO "sys_logininfor"
VALUES (157, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 16:23:59');
INSERT INTO "sys_logininfor"
VALUES (158, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 16:34:15');
INSERT INTO "sys_logininfor"
VALUES (159, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 16:40:58');
INSERT INTO "sys_logininfor"
VALUES (160, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 16:41:27');
INSERT INTO "sys_logininfor"
VALUES (161, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 16:42:09');
INSERT INTO "sys_logininfor"
VALUES (162, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 16:44:27');
INSERT INTO "sys_logininfor"
VALUES (163, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码错误', '2020-06-22 16:54:53');
INSERT INTO "sys_logininfor"
VALUES (164, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 17:00:36');
INSERT INTO "sys_logininfor"
VALUES (165, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-06-22 17:00:46');
INSERT INTO "sys_logininfor"
VALUES (166, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 17:00:57');
INSERT INTO "sys_logininfor"
VALUES (167, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 21:33:01');
INSERT INTO "sys_logininfor"
VALUES (168, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 21:34:25');
INSERT INTO "sys_logininfor"
VALUES (169, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 21:35:18');
INSERT INTO "sys_logininfor"
VALUES (170, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-06-22 21:45:15');
INSERT INTO "sys_logininfor"
VALUES (171, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 21:46:03');
INSERT INTO "sys_logininfor"
VALUES (172, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 21:51:44');
INSERT INTO "sys_logininfor"
VALUES (173, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 21:51:54');
INSERT INTO "sys_logininfor"
VALUES (174, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 21:53:09');
INSERT INTO "sys_logininfor"
VALUES (175, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-22 21:56:13');
INSERT INTO "sys_logininfor"
VALUES (176, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-22 21:56:32');
INSERT INTO "sys_logininfor"
VALUES (177, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-22 22:02:17');
INSERT INTO "sys_logininfor"
VALUES (178, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-22 22:09:27');
INSERT INTO "sys_logininfor"
VALUES (179, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 22:09:57');
INSERT INTO "sys_logininfor"
VALUES (180, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 22:12:49');
INSERT INTO "sys_logininfor"
VALUES (181, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-06-22 22:14:01');
INSERT INTO "sys_logininfor"
VALUES (182, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 22:14:44');
INSERT INTO "sys_logininfor"
VALUES (183, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-06-22 22:16:47');
INSERT INTO "sys_logininfor"
VALUES (184, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 22:17:18');
INSERT INTO "sys_logininfor"
VALUES (185, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-22 22:20:33');
INSERT INTO "sys_logininfor"
VALUES (186, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 09:27:48');
INSERT INTO "sys_logininfor"
VALUES (187, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 09:31:03');
INSERT INTO "sys_logininfor"
VALUES (188, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-23 09:33:14');
INSERT INTO "sys_logininfor"
VALUES (189, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 09:33:21');
INSERT INTO "sys_logininfor"
VALUES (190, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 10:09:26');
INSERT INTO "sys_logininfor"
VALUES (191, 'lihn', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 10:10:17');
INSERT INTO "sys_logininfor"
VALUES (192, 'lin', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 10:10:24');
INSERT INTO "sys_logininfor"
VALUES (193, 'lin', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 10:10:28');
INSERT INTO "sys_logininfor"
VALUES (194, 'lin', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 10:10:58');
INSERT INTO "sys_logininfor"
VALUES (195, 'lin', '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 10:12:20');
INSERT INTO "sys_logininfor"
VALUES (196, 'lin', '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 10:18:51');
INSERT INTO "sys_logininfor"
VALUES (197, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-23 10:19:34');
INSERT INTO "sys_logininfor"
VALUES (198, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 10:19:52');
INSERT INTO "sys_logininfor"
VALUES (199, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 10:40:54');
INSERT INTO "sys_logininfor"
VALUES (200, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 10:41:40');
INSERT INTO "sys_logininfor"
VALUES (201, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 10:43:07');
INSERT INTO "sys_logininfor"
VALUES (202, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 10:43:33');
INSERT INTO "sys_logininfor"
VALUES (203, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 10:44:06');
INSERT INTO "sys_logininfor"
VALUES (204, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 11:20:18');
INSERT INTO "sys_logininfor"
VALUES (205, 123, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 11:26:10');
INSERT INTO "sys_logininfor"
VALUES (206, 'lin', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 11:47:47');
INSERT INTO "sys_logininfor"
VALUES (207, 'lin', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 11:48:14');
INSERT INTO "sys_logininfor"
VALUES (208, 'lin', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误', '2020-06-23 11:48:22');
INSERT INTO "sys_logininfor"
VALUES (209, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 11:48:48');
INSERT INTO "sys_logininfor"
VALUES (210, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-23 13:49:01');
INSERT INTO "sys_logininfor"
VALUES (211, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码错误', '2020-06-23 13:49:02');
INSERT INTO "sys_logininfor"
VALUES (212, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码错误', '2020-06-23 13:49:07');
INSERT INTO "sys_logininfor"
VALUES (213, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 13:49:13');
INSERT INTO "sys_logininfor"
VALUES (214, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 13:49:36');
INSERT INTO "sys_logininfor"
VALUES (215, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 13:50:38');
INSERT INTO "sys_logininfor"
VALUES (216, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 13:51:51');
INSERT INTO "sys_logininfor"
VALUES (217, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 14:02:54');
INSERT INTO "sys_logininfor"
VALUES (218, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 14:10:57');
INSERT INTO "sys_logininfor"
VALUES (219, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 14:23:04');
INSERT INTO "sys_logininfor"
VALUES (220, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 14:25:15');
INSERT INTO "sys_logininfor"
VALUES (221, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-06-23 14:30:36');
INSERT INTO "sys_logininfor"
VALUES (222, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 14:30:53');
INSERT INTO "sys_logininfor"
VALUES (223, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-23 14:32:40');
INSERT INTO "sys_logininfor"
VALUES (224, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 14:32:47');
INSERT INTO "sys_logininfor"
VALUES (225, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-23 14:33:40');
INSERT INTO "sys_logininfor"
VALUES (226, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 14:34:35');
INSERT INTO "sys_logininfor"
VALUES (227, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 14:53:26');
INSERT INTO "sys_logininfor"
VALUES (228, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 14:53:57');
INSERT INTO "sys_logininfor"
VALUES (229, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 14:57:55');
INSERT INTO "sys_logininfor"
VALUES (230, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 14:58:48');
INSERT INTO "sys_logininfor"
VALUES (231, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 14:59:40');
INSERT INTO "sys_logininfor"
VALUES (232, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-23 15:24:55');
INSERT INTO "sys_logininfor"
VALUES (233, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 15:25:05');
INSERT INTO "sys_logininfor"
VALUES (234, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 16:21:45');
INSERT INTO "sys_logininfor"
VALUES (235, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 16:21:52');
INSERT INTO "sys_logininfor"
VALUES (236, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 16:22:56');
INSERT INTO "sys_logininfor"
VALUES (237, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 16:23:06');
INSERT INTO "sys_logininfor"
VALUES (238, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 16:23:34');
INSERT INTO "sys_logininfor"
VALUES (239, 1370832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误',
        '2020-06-23 16:25:03');
INSERT INTO "sys_logininfor"
VALUES (240, 1370832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误',
        '2020-06-23 16:25:12');
INSERT INTO "sys_logininfor"
VALUES (241, 1370832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误',
        '2020-06-23 16:25:28');
INSERT INTO "sys_logininfor"
VALUES (242, 1370832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误',
        '2020-06-23 16:25:30');
INSERT INTO "sys_logininfor"
VALUES (243, 1370832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误',
        '2020-06-23 16:25:38');
INSERT INTO "sys_logininfor"
VALUES (244, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 16:25:51');
INSERT INTO "sys_logininfor"
VALUES (245, 'admin', '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 17:53:01');
INSERT INTO "sys_logininfor"
VALUES (246, 'admin', '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 19:31:46');
INSERT INTO "sys_logininfor"
VALUES (247, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-23 19:39:26');
INSERT INTO "sys_logininfor"
VALUES (248, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-23 20:31:08');
INSERT INTO "sys_logininfor"
VALUES (249, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-24 09:44:29');
INSERT INTO "sys_logininfor"
VALUES (250, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-24 11:19:18');
INSERT INTO "sys_logininfor"
VALUES (251, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-24 13:16:26');
INSERT INTO "sys_logininfor"
VALUES (252, 18022382006, '192.168.0.119', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-24 16:08:49');
INSERT INTO "sys_logininfor"
VALUES (253, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-28 16:11:10');
INSERT INTO "sys_logininfor"
VALUES (254, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-28 17:08:44');
INSERT INTO "sys_logininfor"
VALUES (255, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-28 18:06:41');
INSERT INTO "sys_logininfor"
VALUES (256, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-30 14:39:40');
INSERT INTO "sys_logininfor"
VALUES (257, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-30 15:26:35');
INSERT INTO "sys_logininfor"
VALUES (258, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-30 15:47:38');
INSERT INTO "sys_logininfor"
VALUES (259, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-06-30 16:53:38');
INSERT INTO "sys_logininfor"
VALUES (260, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-30 16:53:45');
INSERT INTO "sys_logininfor"
VALUES (261, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-30 16:59:32');
INSERT INTO "sys_logininfor"
VALUES (262, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-30 17:22:34');
INSERT INTO "sys_logininfor"
VALUES (263, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-30 17:26:46');
INSERT INTO "sys_logininfor"
VALUES (264, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-30 17:26:53');
INSERT INTO "sys_logininfor"
VALUES (265, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-30 17:35:21');
INSERT INTO "sys_logininfor"
VALUES (266, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-06-30 18:57:08');
INSERT INTO "sys_logininfor"
VALUES (267, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-06-30 20:04:35');
INSERT INTO "sys_logininfor"
VALUES (268, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-30 20:04:42');
INSERT INTO "sys_logininfor"
VALUES (269, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-30 20:52:44');
INSERT INTO "sys_logininfor"
VALUES (270, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-30 20:52:58');
INSERT INTO "sys_logininfor"
VALUES (271, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-30 20:54:34');
INSERT INTO "sys_logininfor"
VALUES (272, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-30 20:54:40');
INSERT INTO "sys_logininfor"
VALUES (273, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-30 20:55:55');
INSERT INTO "sys_logininfor"
VALUES (274, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-30 20:56:03');
INSERT INTO "sys_logininfor"
VALUES (275, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-06-30 20:58:32');
INSERT INTO "sys_logininfor"
VALUES (276, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-06-30 20:58:38');
INSERT INTO "sys_logininfor"
VALUES (277, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 10:13:32');
INSERT INTO "sys_logininfor"
VALUES (278, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 10:47:17');
INSERT INTO "sys_logininfor"
VALUES (279, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 11:25:31');
INSERT INTO "sys_logininfor"
VALUES (280, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-01 11:48:50');
INSERT INTO "sys_logininfor"
VALUES (281, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 13:44:41');
INSERT INTO "sys_logininfor"
VALUES (282, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-01 13:45:42');
INSERT INTO "sys_logininfor"
VALUES (283, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 14:29:06');
INSERT INTO "sys_logininfor"
VALUES (284, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 14:29:54');
INSERT INTO "sys_logininfor"
VALUES (285, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-01 14:35:22');
INSERT INTO "sys_logininfor"
VALUES (286, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 14:45:39');
INSERT INTO "sys_logininfor"
VALUES (287, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 16:09:29');
INSERT INTO "sys_logininfor"
VALUES (288, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 16:49:41');
INSERT INTO "sys_logininfor"
VALUES (289, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 17:16:11');
INSERT INTO "sys_logininfor"
VALUES (290, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-01 17:27:31');
INSERT INTO "sys_logininfor"
VALUES (291, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 17:35:26');
INSERT INTO "sys_logininfor"
VALUES (292, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-01 17:37:21');
INSERT INTO "sys_logininfor"
VALUES (293, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-01 17:42:25');
INSERT INTO "sys_logininfor"
VALUES (294, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-01 17:42:32');
INSERT INTO "sys_logininfor"
VALUES (295, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-01 18:01:48');
INSERT INTO "sys_logininfor"
VALUES (296, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-01 18:01:56');
INSERT INTO "sys_logininfor"
VALUES (297, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-07-02 09:47:27');
INSERT INTO "sys_logininfor"
VALUES (298, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-02 09:47:35');
INSERT INTO "sys_logininfor"
VALUES (299, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 09:55:38');
INSERT INTO "sys_logininfor"
VALUES (300, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 13:51:24');
INSERT INTO "sys_logininfor"
VALUES (301, 'admin', '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 14:14:10');
INSERT INTO "sys_logininfor"
VALUES (302, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 14:40:53');
INSERT INTO "sys_logininfor"
VALUES (303, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:28:42');
INSERT INTO "sys_logininfor"
VALUES (304, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-07-02 15:31:12');
INSERT INTO "sys_logininfor"
VALUES (305, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-02 15:31:23');
INSERT INTO "sys_logininfor"
VALUES (306, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-02 15:32:23');
INSERT INTO "sys_logininfor"
VALUES (307, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 1, '验证码错误', '2020-07-02 15:33:32');
INSERT INTO "sys_logininfor"
VALUES (308, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:33:39');
INSERT INTO "sys_logininfor"
VALUES (309, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:33:52');
INSERT INTO "sys_logininfor"
VALUES (310, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:39:16');
INSERT INTO "sys_logininfor"
VALUES (311, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:40:18');
INSERT INTO "sys_logininfor"
VALUES (312, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:41:46');
INSERT INTO "sys_logininfor"
VALUES (313, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:43:20');
INSERT INTO "sys_logininfor"
VALUES (314, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:43:46');
INSERT INTO "sys_logininfor"
VALUES (315, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 1, '验证码错误', '2020-07-02 15:52:54');
INSERT INTO "sys_logininfor"
VALUES (316, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:52:59');
INSERT INTO "sys_logininfor"
VALUES (317, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:58:19');
INSERT INTO "sys_logininfor"
VALUES (318, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 15:58:30');
INSERT INTO "sys_logininfor"
VALUES (319, 'admin', '192.168.0.181', '内网IP', 'Firefox 7', 'Windows 8.1', 0, '登录成功', '2020-07-02 16:05:33');
INSERT INTO "sys_logininfor"
VALUES (320, 'admin', '192.168.0.166', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-02 16:07:21');
INSERT INTO "sys_logininfor"
VALUES (321, 'admin', '192.168.0.166', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 16:12:06');
INSERT INTO "sys_logininfor"
VALUES (322, 18022382006, '192.168.0.166', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 16:28:01');
INSERT INTO "sys_logininfor"
VALUES (323, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 16:30:03');
INSERT INTO "sys_logininfor"
VALUES (324, 'admin', '192.168.0.166', '内网IP', 'Chrome 8', 'Windows 10', 0, '退出成功', '2020-07-02 16:38:49');
INSERT INTO "sys_logininfor"
VALUES (325, 'admin', '192.168.0.166', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 16:38:58');
INSERT INTO "sys_logininfor"
VALUES (326, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 16:39:33');
INSERT INTO "sys_logininfor"
VALUES (327, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-02 16:42:18');
INSERT INTO "sys_logininfor"
VALUES (328, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 16:42:56');
INSERT INTO "sys_logininfor"
VALUES (329, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 16:44:43');
INSERT INTO "sys_logininfor"
VALUES (330, 18022382006, '192.168.0.166', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 17:10:35');
INSERT INTO "sys_logininfor"
VALUES (331, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 17:12:38');
INSERT INTO "sys_logininfor"
VALUES (332, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-02 17:30:09');
INSERT INTO "sys_logininfor"
VALUES (333, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-02 17:30:17');
INSERT INTO "sys_logininfor"
VALUES (334, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 17:30:36');
INSERT INTO "sys_logininfor"
VALUES (335, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-02 17:32:31');
INSERT INTO "sys_logininfor"
VALUES (336, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 19:05:12');
INSERT INTO "sys_logininfor"
VALUES (337, 'admin', '192.168.0.166', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 19:42:48');
INSERT INTO "sys_logininfor"
VALUES (338, 'admin', '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-02 20:23:44');
INSERT INTO "sys_logininfor"
VALUES (339, 'admin', '192.168.0.166', '内网IP', 'Chrome 8', 'Windows 10', 1, '验证码已失效', '2020-07-03 10:16:32');
INSERT INTO "sys_logininfor"
VALUES (340, 13790832772, '192.168.1.108', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-03 10:22:40');
INSERT INTO "sys_logininfor"
VALUES (341, 18022382006, '58.62.30.198', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-03 13:56:29');
INSERT INTO "sys_logininfor"
VALUES (342, 13790832772, '192.168.1.108', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-03 14:23:22');
INSERT INTO "sys_logininfor"
VALUES (343, 13790832772, '192.168.1.108', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-03 14:56:46');
INSERT INTO "sys_logininfor"
VALUES (344, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-03 15:36:47');
INSERT INTO "sys_logininfor"
VALUES (345, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-03 18:45:03');
INSERT INTO "sys_logininfor"
VALUES (346, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-06 11:24:14');
INSERT INTO "sys_logininfor"
VALUES (347, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-06 11:30:07');
INSERT INTO "sys_logininfor"
VALUES (348, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-06 13:59:07');
INSERT INTO "sys_logininfor"
VALUES (349, 'admin', '192.168.0.166', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-06 15:43:09');
INSERT INTO "sys_logininfor"
VALUES (350, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-06 16:00:55');
INSERT INTO "sys_logininfor"
VALUES (351, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-07-07 10:13:10');
INSERT INTO "sys_logininfor"
VALUES (352, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 10:13:15');
INSERT INTO "sys_logininfor"
VALUES (353, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 10:25:18');
INSERT INTO "sys_logininfor"
VALUES (354, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 11:15:38');
INSERT INTO "sys_logininfor"
VALUES (355, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 11:46:42');
INSERT INTO "sys_logininfor"
VALUES (356, 18022382006, '192.168.0.166', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-07 12:43:02');
INSERT INTO "sys_logininfor"
VALUES (357, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 13:56:05');
INSERT INTO "sys_logininfor"
VALUES (358, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 15:24:06');
INSERT INTO "sys_logininfor"
VALUES (359, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 16:26:51');
INSERT INTO "sys_logininfor"
VALUES (360, 'admin', '192.168.0.166', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-07 16:30:49');
INSERT INTO "sys_logininfor"
VALUES (361, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 17:47:19');
INSERT INTO "sys_logininfor"
VALUES (362, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 18:50:49');
INSERT INTO "sys_logininfor"
VALUES (363, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 19:43:20');
INSERT INTO "sys_logininfor"
VALUES (364, 'admin', '192.168.0.166', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-07 20:32:48');
INSERT INTO "sys_logininfor"
VALUES (365, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-07 20:36:07');
INSERT INTO "sys_logininfor"
VALUES (366, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 09:23:16');
INSERT INTO "sys_logininfor"
VALUES (367, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 10:20:12');
INSERT INTO "sys_logininfor"
VALUES (368, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 10:27:26');
INSERT INTO "sys_logininfor"
VALUES (369, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-08 10:45:56');
INSERT INTO "sys_logininfor"
VALUES (370, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-08 10:49:14');
INSERT INTO "sys_logininfor"
VALUES (371, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-08 10:49:21');
INSERT INTO "sys_logininfor"
VALUES (372, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 10:52:25');
INSERT INTO "sys_logininfor"
VALUES (373, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-08 11:08:34');
INSERT INTO "sys_logininfor"
VALUES (374, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-08 11:08:43');
INSERT INTO "sys_logininfor"
VALUES (375, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-08 11:37:49');
INSERT INTO "sys_logininfor"
VALUES (376, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-08 11:37:56');
INSERT INTO "sys_logininfor"
VALUES (377, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 11:55:10');
INSERT INTO "sys_logininfor"
VALUES (378, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-08 12:02:36');
INSERT INTO "sys_logininfor"
VALUES (379, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 13:49:01');
INSERT INTO "sys_logininfor"
VALUES (380, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-08 13:55:23');
INSERT INTO "sys_logininfor"
VALUES (381, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-07-08 14:46:49');
INSERT INTO "sys_logininfor"
VALUES (382, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 14:46:58');
INSERT INTO "sys_logininfor"
VALUES (383, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 17:02:25');
INSERT INTO "sys_logininfor"
VALUES (384, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 17:30:20');
INSERT INTO "sys_logininfor"
VALUES (385, 18022382006, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-08 18:21:51');
INSERT INTO "sys_logininfor"
VALUES (386, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 09:13:06');
INSERT INTO "sys_logininfor"
VALUES (387, 13333333333, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 09:37:51');
INSERT INTO "sys_logininfor"
VALUES (388, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 09:50:06');
INSERT INTO "sys_logininfor"
VALUES (389, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 10:02:29');
INSERT INTO "sys_logininfor"
VALUES (390, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-09 10:12:41');
INSERT INTO "sys_logininfor"
VALUES (391, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-07-09 11:05:47');
INSERT INTO "sys_logininfor"
VALUES (392, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 11:06:12');
INSERT INTO "sys_logininfor"
VALUES (393, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 11:36:44');
INSERT INTO "sys_logininfor"
VALUES (394, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 11:41:38');
INSERT INTO "sys_logininfor"
VALUES (395, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 13:57:53');
INSERT INTO "sys_logininfor"
VALUES (396, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-09 14:33:17');
INSERT INTO "sys_logininfor"
VALUES (397, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-07-09 18:08:12');
INSERT INTO "sys_logininfor"
VALUES (398, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 18:08:18');
INSERT INTO "sys_logininfor"
VALUES (399, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 19:00:09');
INSERT INTO "sys_logininfor"
VALUES (400, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-09 19:13:32');
INSERT INTO "sys_logininfor"
VALUES (401, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-09 19:16:40');
INSERT INTO "sys_logininfor"
VALUES (402, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-09 19:19:19');
INSERT INTO "sys_logininfor"
VALUES (403, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-09 19:27:31');
INSERT INTO "sys_logininfor"
VALUES (404, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-09 19:27:57');
INSERT INTO "sys_logininfor"
VALUES (405, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 09:18:53');
INSERT INTO "sys_logininfor"
VALUES (406, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 15:10:05');
INSERT INTO "sys_logininfor"
VALUES (407, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 15:12:36');
INSERT INTO "sys_logininfor"
VALUES (408, 18022302006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户名或密码错误',
        '2020-07-10 15:28:35');
INSERT INTO "sys_logininfor"
VALUES (409, 18022302006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '用户名或密码错误',
        '2020-07-10 15:28:41');
INSERT INTO "sys_logininfor"
VALUES (410, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 15:28:51');
INSERT INTO "sys_logininfor"
VALUES (411, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 16:17:57');
INSERT INTO "sys_logininfor"
VALUES (412, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 16:53:00');
INSERT INTO "sys_logininfor"
VALUES (413, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 17:22:43');
INSERT INTO "sys_logininfor"
VALUES (414, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 21:34:13');
INSERT INTO "sys_logininfor"
VALUES (415, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-10 21:59:26');
INSERT INTO "sys_logininfor"
VALUES (416, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-10 21:59:38');
INSERT INTO "sys_logininfor"
VALUES (417, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-14 11:44:53');
INSERT INTO "sys_logininfor"
VALUES (418, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-14 11:45:06');
INSERT INTO "sys_logininfor"
VALUES (419, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-14 17:24:57');
INSERT INTO "sys_logininfor"
VALUES (420, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-14 19:36:34');
INSERT INTO "sys_logininfor"
VALUES (421, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-14 19:44:27');
INSERT INTO "sys_logininfor"
VALUES (422, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-14 19:57:17');
INSERT INTO "sys_logininfor"
VALUES (423, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-15 09:23:54');
INSERT INTO "sys_logininfor"
VALUES (424, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 10:37:37');
INSERT INTO "sys_logininfor"
VALUES (425, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 10:39:06');
INSERT INTO "sys_logininfor"
VALUES (426, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 11:14:37');
INSERT INTO "sys_logininfor"
VALUES (427, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 11:14:44');
INSERT INTO "sys_logininfor"
VALUES (428, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户不存在/密码错误',
        '2020-07-17 11:32:53');
INSERT INTO "sys_logininfor"
VALUES (429, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 11:33:02');
INSERT INTO "sys_logininfor"
VALUES (430, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 14:33:34');
INSERT INTO "sys_logininfor"
VALUES (431, 12790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误',
        '2020-07-17 16:16:02');
INSERT INTO "sys_logininfor"
VALUES (432, 12790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 1, '用户名或密码错误',
        '2020-07-17 16:16:08');
INSERT INTO "sys_logininfor"
VALUES (433, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 16:16:20');
INSERT INTO "sys_logininfor"
VALUES (434, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-17 16:31:39');
INSERT INTO "sys_logininfor"
VALUES (435, 13790832772, '127.0.0.1', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 09:30:06');
INSERT INTO "sys_logininfor"
VALUES (436, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 09:40:29');
INSERT INTO "sys_logininfor"
VALUES (437, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 10:52:49');
INSERT INTO "sys_logininfor"
VALUES (438, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 11:27:21');
INSERT INTO "sys_logininfor"
VALUES (439, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 11:45:57');
INSERT INTO "sys_logininfor"
VALUES (440, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 13:43:17');
INSERT INTO "sys_logininfor"
VALUES (441, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 17:00:45');
INSERT INTO "sys_logininfor"
VALUES (442, 'admin', '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-20 17:31:50');
INSERT INTO "sys_logininfor"
VALUES (443, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 09:17:16');
INSERT INTO "sys_logininfor"
VALUES (444, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 09:58:51');
INSERT INTO "sys_logininfor"
VALUES (445, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 10:09:18');
INSERT INTO "sys_logininfor"
VALUES (446, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 10:17:49');
INSERT INTO "sys_logininfor"
VALUES (447, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 11:59:59');
INSERT INTO "sys_logininfor"
VALUES (448, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 12:01:30');
INSERT INTO "sys_logininfor"
VALUES (449, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 13:47:49');
INSERT INTO "sys_logininfor"
VALUES (450, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 14:05:13');
INSERT INTO "sys_logininfor"
VALUES (451, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 14:22:15');
INSERT INTO "sys_logininfor"
VALUES (452, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 14:54:08');
INSERT INTO "sys_logininfor"
VALUES (453, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 14:54:33');
INSERT INTO "sys_logininfor"
VALUES (454, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 14:54:40');
INSERT INTO "sys_logininfor"
VALUES (455, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 15:01:47');
INSERT INTO "sys_logininfor"
VALUES (456, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 15:04:43');
INSERT INTO "sys_logininfor"
VALUES (457, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 15:17:19');
INSERT INTO "sys_logininfor"
VALUES (458, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 15:18:26');
INSERT INTO "sys_logininfor"
VALUES (459, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 15:25:58');
INSERT INTO "sys_logininfor"
VALUES (460, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 15:45:09');
INSERT INTO "sys_logininfor"
VALUES (461, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 15:52:12');
INSERT INTO "sys_logininfor"
VALUES (462, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 15:56:24');
INSERT INTO "sys_logininfor"
VALUES (463, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 16:06:46');
INSERT INTO "sys_logininfor"
VALUES (464, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 16:23:14');
INSERT INTO "sys_logininfor"
VALUES (465, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 16:23:57');
INSERT INTO "sys_logininfor"
VALUES (466, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 17:23:04');
INSERT INTO "sys_logininfor"
VALUES (467, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 17:27:33');
INSERT INTO "sys_logininfor"
VALUES (468, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 17:27:41');
INSERT INTO "sys_logininfor"
VALUES (469, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 17:30:32');
INSERT INTO "sys_logininfor"
VALUES (470, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 17:30:40');
INSERT INTO "sys_logininfor"
VALUES (471, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 17:38:17');
INSERT INTO "sys_logininfor"
VALUES (472, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 17:38:25');
INSERT INTO "sys_logininfor"
VALUES (473, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 17:41:21');
INSERT INTO "sys_logininfor"
VALUES (474, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 17:46:40');
INSERT INTO "sys_logininfor"
VALUES (475, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 17:46:48');
INSERT INTO "sys_logininfor"
VALUES (476, 18022382006, '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 17:56:56');
INSERT INTO "sys_logininfor"
VALUES (477, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 17:58:22');
INSERT INTO "sys_logininfor"
VALUES (478, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-21 18:48:35');
INSERT INTO "sys_logininfor"
VALUES (479, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 18:54:15');
INSERT INTO "sys_logininfor"
VALUES (480, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 18:56:22');
INSERT INTO "sys_logininfor"
VALUES (481, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 19:07:04');
INSERT INTO "sys_logininfor"
VALUES (482, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码错误', '2020-07-21 19:07:14');
INSERT INTO "sys_logininfor"
VALUES (483, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 19:07:19');
INSERT INTO "sys_logininfor"
VALUES (484, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 19:09:19');
INSERT INTO "sys_logininfor"
VALUES (485, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-21 19:09:36');
INSERT INTO "sys_logininfor"
VALUES (486, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 19:09:43');
INSERT INTO "sys_logininfor"
VALUES (487, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-21 20:03:50');
INSERT INTO "sys_logininfor"
VALUES (488, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-22 09:01:52');
INSERT INTO "sys_logininfor"
VALUES (489, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-22 09:52:13');
INSERT INTO "sys_logininfor"
VALUES (490, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-22 10:46:16');
INSERT INTO "sys_logininfor"
VALUES (491, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-22 10:54:35');
INSERT INTO "sys_logininfor"
VALUES (492, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-22 11:18:05');
INSERT INTO "sys_logininfor"
VALUES (493, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-22 13:43:19');
INSERT INTO "sys_logininfor"
VALUES (494, 'admin', '192.168.0.167', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-22 15:28:58');
INSERT INTO "sys_logininfor"
VALUES (495, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-22 15:50:58');
INSERT INTO "sys_logininfor"
VALUES (496, 13790832772, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-22 16:37:35');
INSERT INTO "sys_logininfor"
VALUES (497, 18022382006, '192.168.0.128', '内网IP', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-23 10:26:54');
INSERT INTO "sys_logininfor"
VALUES (498, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-07-24 10:03:56');
INSERT INTO "sys_logininfor"
VALUES (499, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-24 10:04:02');
INSERT INTO "sys_logininfor"
VALUES (500, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-24 10:33:28');
INSERT INTO "sys_logininfor"
VALUES (501, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-24 10:33:37');
INSERT INTO "sys_logininfor"
VALUES (502, 'admin', '113.115.54.231', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 11:46:18');
INSERT INTO "sys_logininfor"
VALUES (503, 18022382006, '113.115.54.231', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 11:48:38');
INSERT INTO "sys_logininfor"
VALUES (504, 'admin', '58.62.31.240', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 13:47:40');
INSERT INTO "sys_logininfor"
VALUES (505, 'admin', '58.62.31.240', 'XX XX', 'Chrome 8', 'Windows 10', 0, '退出成功', '2020-07-24 13:48:19');
INSERT INTO "sys_logininfor"
VALUES (506, 'admin', '58.62.31.240', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 13:48:26');
INSERT INTO "sys_logininfor"
VALUES (507, 'admin', '58.62.31.240', 'XX XX', 'Chrome 8', 'Windows 10', 0, '退出成功', '2020-07-24 13:51:37');
INSERT INTO "sys_logininfor"
VALUES (508, 'admin', '58.62.31.240', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 13:51:51');
INSERT INTO "sys_logininfor"
VALUES (509, 'admin', '58.62.31.240', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-24 13:52:11');
INSERT INTO "sys_logininfor"
VALUES (510, 18022382006, '113.115.54.231', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 14:08:41');
INSERT INTO "sys_logininfor"
VALUES (511, 18022382006, '113.115.54.231', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 14:11:42');
INSERT INTO "sys_logininfor"
VALUES (512, 'admin', '113.115.54.231', 'XX XX', 'Chrome 8', 'Windows 10', 0, '登录成功', '2020-07-24 14:40:50');
INSERT INTO "sys_logininfor"
VALUES (513, 18073721771, '58.62.31.156', 'XX XX', 'Firefox 7', 'Windows 10', 1, '用户名或密码错误',
        '2020-07-26 09:50:13');
INSERT INTO "sys_logininfor"
VALUES (514, 18022382006, '58.62.31.156', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-26 09:50:25');
INSERT INTO "sys_logininfor"
VALUES (515, 18022382006, '14.24.151.46', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-26 10:29:13');
INSERT INTO "sys_logininfor"
VALUES (516, 18073721771, '14.24.151.46', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-26 10:34:31');
INSERT INTO "sys_logininfor"
VALUES (517, 'admin', '14.24.151.46', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-26 10:35:47');
INSERT INTO "sys_logininfor"
VALUES (518, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-26 17:48:31');
INSERT INTO "sys_logininfor"
VALUES (519, 18022382006, '58.62.29.238', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-27 15:07:22');
INSERT INTO "sys_logininfor"
VALUES (520, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 1, '验证码已失效', '2020-07-27 17:07:35');
INSERT INTO "sys_logininfor"
VALUES (521, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-27 17:07:42');
INSERT INTO "sys_logininfor"
VALUES (522, 'admin', '58.62.29.238', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-27 17:56:21');
INSERT INTO "sys_logininfor"
VALUES (523, 18022382006, '58.62.29.238', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-27 17:57:59');
INSERT INTO "sys_logininfor"
VALUES (524, 'admin', '58.62.29.238', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 11:42:02');
INSERT INTO "sys_logininfor"
VALUES (525, 'admin', '58.62.29.238', 'XX XX', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-28 11:46:32');
INSERT INTO "sys_logininfor"
VALUES (526, 'admin', '58.62.29.238', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 11:46:43');
INSERT INTO "sys_logininfor"
VALUES (527, 'admin', '58.62.29.238', 'XX XX', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 14:05:18');
INSERT INTO "sys_logininfor"
VALUES (528, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 15:32:14');
INSERT INTO "sys_logininfor"
VALUES (529, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 15:38:03');
INSERT INTO "sys_logininfor"
VALUES (530, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 15:40:15');
INSERT INTO "sys_logininfor"
VALUES (531, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '退出成功', '2020-07-28 15:43:59');
INSERT INTO "sys_logininfor"
VALUES (532, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 15:44:15');
INSERT INTO "sys_logininfor"
VALUES (533, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 15:44:56');
INSERT INTO "sys_logininfor"
VALUES (534, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 15:47:39');
INSERT INTO "sys_logininfor"
VALUES (535, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 16:30:37');
INSERT INTO "sys_logininfor"
VALUES (536, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 16:31:49');
INSERT INTO "sys_logininfor"
VALUES (537, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 16:44:06');
INSERT INTO "sys_logininfor"
VALUES (538, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 16:45:42');
INSERT INTO "sys_logininfor"
VALUES (539, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 17:12:15');
INSERT INTO "sys_logininfor"
VALUES (540, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 17:27:24');
INSERT INTO "sys_logininfor"
VALUES (541, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 17:34:44');
INSERT INTO "sys_logininfor"
VALUES (542, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 17:39:37');
INSERT INTO "sys_logininfor"
VALUES (543, 18022382006, '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 17:43:57');
INSERT INTO "sys_logininfor"
VALUES (544, 'admin', '127.0.0.1', '内网IP', 'Firefox 7', 'Windows 10', 0, '登录成功', '2020-07-28 17:45:03');

-- ----------------------------
-- Table structure for sys_menu
-- ----------------------------
DROP TABLE IF EXISTS "sys_menu";
CREATE TABLE "sys_menu"
(
    "menu_id"     integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "menu_name"   text(50) NOT NULL,
    "parent_id"   integer(20),
    "order_num"   integer(11),
    "path"        text(200),
    "component"   text(255),
    "is_frame"    integer(11),
    "menu_type"   text(1),
    "visible"     text(1),
    "status"      text(1),
    "perms"       text(100),
    "icon"        text(100),
    "create_by"   text(64),
    "create_time" text,
    "update_by"   text(64),
    "update_time" text,
    "remark"      text(500)
);

-- ----------------------------
-- Records of sys_menu
-- ----------------------------
INSERT INTO "sys_menu"
VALUES (1, '系统管理', 0, 1, 'system', NULL, 1, 'M', 0, 0, '', 'system', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '系统管理目录');
INSERT INTO "sys_menu"
VALUES (2, '系统监控', 0, 2, 'monitor', NULL, 1, 'M', 0, 0, '', 'monitor', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '系统监控目录');
INSERT INTO "sys_menu"
VALUES (3, '系统工具', 0, 3, 'tool', NULL, 1, 'M', 0, 0, '', 'tool', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '系统工具目录');
INSERT INTO "sys_menu"
VALUES (4, '会员管理', 0, 4, 'dq', NULL, 1, 'M', 0, 0, '', 'guide', 'admin', '2018-03-16 11:33:00', 'admin',
        '2020-06-20 10:02:25', '若依官网地址');
INSERT INTO "sys_menu"
VALUES (100, '用户管理', 1, 1, 'user', 'system/user/index', 1, 'C', 0, 0, 'system:user:list', 'user', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '用户管理菜单');
INSERT INTO "sys_menu"
VALUES (101, '角色管理', 1, 2, 'role', 'system/role/index', 1, 'C', 0, 0, 'system:role:list', 'peoples', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '角色管理菜单');
INSERT INTO "sys_menu"
VALUES (102, '菜单管理', 1, 3, 'menu', 'system/menu/index', 1, 'C', 0, 0, 'system:menu:list', 'tree-table', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '菜单管理菜单');
INSERT INTO "sys_menu"
VALUES (103, '部门管理', 1, 4, 'dept', 'system/dept/index', 1, 'C', 0, 0, 'system:dept:list', 'tree', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '部门管理菜单');
INSERT INTO "sys_menu"
VALUES (104, '岗位管理', 1, 5, 'post', 'system/post/index', 1, 'C', 0, 0, 'system:post:list', 'post', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '岗位管理菜单');
INSERT INTO "sys_menu"
VALUES (105, '字典管理', 1, 6, 'dict', 'system/dict/index', 1, 'C', 0, 0, 'system:dict:list', 'dict', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '字典管理菜单');
INSERT INTO "sys_menu"
VALUES (106, '参数设置', 1, 7, 'config', 'system/config/index', 1, 'C', 0, 0, 'system:config:list', 'edit', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '参数设置菜单');
INSERT INTO "sys_menu"
VALUES (107, '通知公告', 1, 8, 'notice', 'system/notice/index', 1, 'C', 0, 0, 'system:notice:list', 'message', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '通知公告菜单');
INSERT INTO "sys_menu"
VALUES (108, '日志管理', 1, 9, 'log', 'system/log/index', 1, 'M', 0, 0, '', 'log', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '日志管理菜单');
INSERT INTO "sys_menu"
VALUES (109, '在线用户', 2, 1, 'online', 'monitor/online/index', 1, 'C', 0, 0, 'monitor:online:list', 'online', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '在线用户菜单');
INSERT INTO "sys_menu"
VALUES (110, '定时任务', 2, 2, 'job', 'monitor/job/index', 1, 'C', 0, 0, 'monitor:job:list', 'job', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '定时任务菜单');
INSERT INTO "sys_menu"
VALUES (111, '数据监控', 2, 3, 'druid', 'monitor/druid/index', 1, 'C', 0, 0, 'monitor:druid:list', 'druid', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '数据监控菜单');
INSERT INTO "sys_menu"
VALUES (112, '服务监控', 2, 4, 'server', 'monitor/server/index', 1, 'C', 0, 0, 'monitor:server:list', 'server', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '服务监控菜单');
INSERT INTO "sys_menu"
VALUES (113, '表单构建', 3, 1, 'build', 'tool/build/index', 1, 'C', 0, 0, 'tool:build:list', 'build', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '表单构建菜单');
INSERT INTO "sys_menu"
VALUES (114, '代码生成', 3, 2, 'gen', 'tool/gen/index', 1, 'C', 0, 0, 'tool:gen:list', 'code', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '代码生成菜单');
INSERT INTO "sys_menu"
VALUES (115, '系统接口', 3, 3, 'swagger', 'tool/swagger/index', 1, 'C', 0, 0, 'tool:swagger:list', 'swagger', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '系统接口菜单');
INSERT INTO "sys_menu"
VALUES (500, '操作日志', 108, 1, 'operlog', 'monitor/operlog/index', 1, 'C', 0, 0, 'monitor:operlog:list', 'form',
        'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '操作日志菜单');
INSERT INTO "sys_menu"
VALUES (501, '登录日志', 108, 2, 'logininfor', 'monitor/logininfor/index', 1, 'C', 0, 0, 'monitor:logininfor:list',
        'logininfor', 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '登录日志菜单');
INSERT INTO "sys_menu"
VALUES (1001, '用户查询', 100, 1, '', '', 1, 'F', 0, 0, 'system:user:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1002, '用户新增', 100, 2, '', '', 1, 'F', 0, 0, 'system:user:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1003, '用户修改', 100, 3, '', '', 1, 'F', 0, 0, 'system:user:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1004, '用户删除', 100, 4, '', '', 1, 'F', 0, 0, 'system:user:remove', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1005, '用户导出', 100, 5, '', '', 1, 'F', 0, 0, 'system:user:export', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1006, '用户导入', 100, 6, '', '', 1, 'F', 0, 0, 'system:user:import', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1007, '重置密码', 100, 7, '', '', 1, 'F', 0, 0, 'system:user:resetPwd', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1008, '角色查询', 101, 1, '', '', 1, 'F', 0, 0, 'system:role:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1009, '角色新增', 101, 2, '', '', 1, 'F', 0, 0, 'system:role:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1010, '角色修改', 101, 3, '', '', 1, 'F', 0, 0, 'system:role:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1011, '角色删除', 101, 4, '', '', 1, 'F', 0, 0, 'system:role:remove', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1012, '角色导出', 101, 5, '', '', 1, 'F', 0, 0, 'system:role:export', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1013, '菜单查询', 102, 1, '', '', 1, 'F', 0, 0, 'system:menu:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1014, '菜单新增', 102, 2, '', '', 1, 'F', 0, 0, 'system:menu:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1015, '菜单修改', 102, 3, '', '', 1, 'F', 0, 0, 'system:menu:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1016, '菜单删除', 102, 4, '', '', 1, 'F', 0, 0, 'system:menu:remove', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1017, '部门查询', 103, 1, '', '', 1, 'F', 0, 0, 'system:dept:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1018, '部门新增', 103, 2, '', '', 1, 'F', 0, 0, 'system:dept:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1019, '部门修改', 103, 3, '', '', 1, 'F', 0, 0, 'system:dept:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1020, '部门删除', 103, 4, '', '', 1, 'F', 0, 0, 'system:dept:remove', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1021, '岗位查询', 104, 1, '', '', 1, 'F', 0, 0, 'system:post:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1022, '岗位新增', 104, 2, '', '', 1, 'F', 0, 0, 'system:post:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1023, '岗位修改', 104, 3, '', '', 1, 'F', 0, 0, 'system:post:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1024, '岗位删除', 104, 4, '', '', 1, 'F', 0, 0, 'system:post:remove', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1025, '岗位导出', 104, 5, '', '', 1, 'F', 0, 0, 'system:post:export', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1026, '字典查询', 105, 1, '#', '', 1, 'F', 0, 0, 'system:dict:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1027, '字典新增', 105, 2, '#', '', 1, 'F', 0, 0, 'system:dict:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1028, '字典修改', 105, 3, '#', '', 1, 'F', 0, 0, 'system:dict:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1029, '字典删除', 105, 4, '#', '', 1, 'F', 0, 0, 'system:dict:remove', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1030, '字典导出', 105, 5, '#', '', 1, 'F', 0, 0, 'system:dict:export', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1031, '参数查询', 106, 1, '#', '', 1, 'F', 0, 0, 'system:config:query', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1032, '参数新增', 106, 2, '#', '', 1, 'F', 0, 0, 'system:config:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1033, '参数修改', 106, 3, '#', '', 1, 'F', 0, 0, 'system:config:edit', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1034, '参数删除', 106, 4, '#', '', 1, 'F', 0, 0, 'system:config:remove', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1035, '参数导出', 106, 5, '#', '', 1, 'F', 0, 0, 'system:config:export', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1036, '公告查询', 107, 1, '#', '', 1, 'F', 0, 0, 'system:notice:query', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1037, '公告新增', 107, 2, '#', '', 1, 'F', 0, 0, 'system:notice:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1038, '公告修改', 107, 3, '#', '', 1, 'F', 0, 0, 'system:notice:edit', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1039, '公告删除', 107, 4, '#', '', 1, 'F', 0, 0, 'system:notice:remove', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1040, '操作查询', 500, 1, '#', '', 1, 'F', 0, 0, 'monitor:operlog:query', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1041, '操作删除', 500, 2, '#', '', 1, 'F', 0, 0, 'monitor:operlog:remove', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1042, '日志导出', 500, 4, '#', '', 1, 'F', 0, 0, 'monitor:operlog:export', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1043, '登录查询', 501, 1, '#', '', 1, 'F', 0, 0, 'monitor:logininfor:query', '#', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1044, '登录删除', 501, 2, '#', '', 1, 'F', 0, 0, 'monitor:logininfor:remove', '#', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1045, '日志导出', 501, 3, '#', '', 1, 'F', 0, 0, 'monitor:logininfor:export', '#', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1046, '在线查询', 109, 1, '#', '', 1, 'F', 0, 0, 'monitor:online:query', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1047, '批量强退', 109, 2, '#', '', 1, 'F', 0, 0, 'monitor:online:batchLogout', '#', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1048, '单条强退', 109, 3, '#', '', 1, 'F', 0, 0, 'monitor:online:forceLogout', '#', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1049, '任务查询', 110, 1, '#', '', 1, 'F', 0, 0, 'monitor:job:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1050, '任务新增', 110, 2, '#', '', 1, 'F', 0, 0, 'monitor:job:add', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1051, '任务修改', 110, 3, '#', '', 1, 'F', 0, 0, 'monitor:job:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1052, '任务删除', 110, 4, '#', '', 1, 'F', 0, 0, 'monitor:job:remove', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1053, '状态修改', 110, 5, '#', '', 1, 'F', 0, 0, 'monitor:job:changeStatus', '#', 'admin',
        '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1054, '任务导出', 110, 7, '#', '', 1, 'F', 0, 0, 'monitor:job:export', '#', 'admin', '2018-03-16 11:33:00',
        'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1055, '生成查询', 114, 1, '#', '', 1, 'F', 0, 0, 'tool:gen:query', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1056, '生成修改', 114, 2, '#', '', 1, 'F', 0, 0, 'tool:gen:edit', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1057, '生成删除', 114, 3, '#', '', 1, 'F', 0, 0, 'tool:gen:remove', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1058, '导入代码', 114, 2, '#', '', 1, 'F', 0, 0, 'tool:gen:import', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1059, '预览代码', 114, 4, '#', '', 1, 'F', 0, 0, 'tool:gen:preview', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (1060, '生成代码', 114, 5, '#', '', 1, 'F', 0, 0, 'tool:gen:code', '#', 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '');
INSERT INTO "sys_menu"
VALUES (2000, '会员管理', 4, 1, 'appuser', 'dq/appuser/index', 1, 'C', 0, 0, 'system:appUser:appUser', 'peoples',
        'admin', '2020-06-20 10:03:52', 'admin', '2020-07-28 11:43:13', '');
INSERT INTO "sys_menu"
VALUES (2001, '充值管理', 4, 2, 'recharge', 'dq/recharge/index', 1, 'C', 0, 0, 'system:recharge:recharge', 'money',
        'admin', '2020-06-20 15:33:46', 'admin', '2020-07-28 11:43:37', '');
INSERT INTO "sys_menu"
VALUES (2002, '打标管理', 0, 5, 'dqmarking', NULL, 1, 'M', 0, 0, '', 'build', 'admin', '2020-06-21 10:31:10', 'admin',
        '2020-06-21 15:22:31', '');
INSERT INTO "sys_menu"
VALUES (2005, '统计管理', 0, 6, 'stastitac', NULL, 1, 'M', 0, 0, '', 'code', 'admin', '2020-06-21 15:15:27', 'admin',
        '2020-06-21 15:22:41', '');
INSERT INTO "sys_menu"
VALUES (2006, '验号统计', 2005, 1, 'yhstatistics', 'dq/statistics/yhindex', 1, 'C', 0, 0,
        'system:dsjfstatistics:yhlist', 'monitor', 'admin', '2020-06-21 15:16:20', 'admin', '2020-07-28 11:45:54', '');
INSERT INTO "sys_menu"
VALUES (2007, '流量统计', 2005, 2, 'flowstatistics', 'dq/statistics/flowindex', 1, 'C', 0, 0,
        'system:dsjfstatistics:flowlist', 'component', 'admin', '2020-06-21 15:17:09', 'admin', '2020-07-28 11:46:04',
        '');
INSERT INTO "sys_menu"
VALUES (2008, '派大淘流量统计', 2005, 3, 'pdtstatistics', 'dq/statistics/pdtflowindex', 1, 'C', 0, 0,
        'system:pdtstatistics:flowlist', 'message', 'admin', '2020-06-30 17:25:36', 'admin', '2020-07-28 11:46:11', '');
INSERT INTO "sys_menu"
VALUES (2009, '接口管理', 4, 3, 'callrecord', 'dq/callrecord/index', 1, 'C', 0, 0, 'system:callrecord:list', 'server',
        'admin', '2020-06-30 20:54:30', 'admin', '2020-07-28 11:44:04', '');
INSERT INTO "sys_menu"
VALUES (2010, '门户网站在线用户', 2, 1, 'webonline', 'monitor/webonline/index', 1, 'C', 0, 0, 'monitor:webonline:list',
        'user', 'admin', '2020-07-02 17:29:47', 'admin', '2020-07-02 17:30:48', '');
INSERT INTO "sys_menu"
VALUES (2011, '打标统计', 2005, 4, 'makinglist', 'dq/statistics/dbindex', 1, 'C', 0, 0,
        'system:dsjfstatistics:makinglist', 'nested', 'admin', '2020-07-08 10:48:02', 'admin', '2020-07-28 11:46:19',
        '');
INSERT INTO "sys_menu"
VALUES (2012, '打标记录', 2002, 1, 'markingRecord', 'dq/marking/index', 1, 'C', 0, 0,
        'system:markingRecord:markingRecord', 'list', 'admin', '2020-07-08 11:08:27', 'admin', '2020-07-28 11:45:05',
        '');
INSERT INTO "sys_menu"
VALUES (2013, '验号记录', 2002, 2, 'yhrecord', 'dq/yhrecord/index', 1, 'C', 0, 0, 'system:yhrecord:list', 'language',
        'admin', '2020-07-08 11:35:59', 'admin', '2020-07-28 11:45:16', '');
INSERT INTO "sys_menu"
VALUES (2014, '电商教父流量记录', 2002, 3, 'flowrecord', 'dq/flowrecord/index', 1, 'C', 0, 0, 'system:flowrecord:list',
        'international', 'admin', '2020-07-08 11:36:46', 'admin', '2020-07-28 11:45:31', '');
INSERT INTO "sys_menu"
VALUES (2015, '派大淘流量记录', 2002, 4, 'pdtflow', 'dq/pdtflow/index', 1, 'C', 0, 0, 'system:pdtflow:list', 'job',
        'admin', '2020-07-08 11:37:44', 'admin', '2020-07-28 11:45:41', '');

-- ----------------------------
-- Table structure for sys_notice
-- ----------------------------
DROP TABLE IF EXISTS "sys_notice";
CREATE TABLE "sys_notice"
(
    "notice_id"      integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "notice_title"   text(50) NOT NULL,
    "notice_type"    text(1) NOT NULL,
    "notice_content" text(2000),
    "status"         text(1),
    "create_by"      text(64),
    "create_time"    text,
    "update_by"      text(64),
    "update_time"    text,
    "remark"         text(255)
);

-- ----------------------------
-- Records of sys_notice
-- ----------------------------
INSERT INTO "sys_notice"
VALUES (1, '温馨提醒：2018-07-01 若依新版本发布啦', 2, '新版本内容', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '管理员');
INSERT INTO "sys_notice"
VALUES (2, '维护通知：2018-07-01 若依系统凌晨维护', 1, '维护内容', 0, 'admin', '2018-03-16 11:33:00', 'ry',
        '2018-03-16 11:33:00', '管理员');
INSERT INTO "sys_notice"
VALUES (3, '双11加购通知', 1, '<p>双11加购通知</p>', 0, 'admin', '2020-07-06 15:43:58', '', NULL, NULL);

-- ----------------------------
-- Table structure for sys_oper_log
-- ----------------------------
DROP TABLE IF EXISTS "sys_oper_log";
CREATE TABLE "sys_oper_log"
(
    "oper_id"        integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "title"          text(50),
    "business_type"  integer(11),
    "method"         text(100),
    "request_method" text(10),
    "operator_type"  integer(11),
    "oper_name"      text(50),
    "dept_name"      text(50),
    "oper_url"       text(255),
    "oper_ip"        text(50),
    "oper_location"  text(255),
    "oper_param"     text(2000),
    "json_result"    text(2000),
    "status"         integer(11),
    "error_msg"      text(2000),
    "oper_time"      text
);

-- ----------------------------
-- Records of sys_oper_log
-- ----------------------------
INSERT INTO "sys_oper_log"
VALUES (100, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/06/10/d530e3649052d39f03993b96a53a7477.jpeg","code":200}', 0,
        NULL, '2020-06-10 16:31:19');
INSERT INTO "sys_oper_log"
VALUES (101, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '127.0.0.1', '内网IP',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15888888888","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$7JB720yubVSZvUI0rEqK/.VqGOZTH.ulu33dHOiBE8ByOhJIrdAu2","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"若依","sex":"1","deptId":103,"avatar":"","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-10 16:31:22');
INSERT INTO "sys_oper_log"
VALUES (102, '代码生成', 6, 'com.ruoyi.project.tool.gen.controller.GenController.importTableSave()', 'POST', 1, 'admin',
        NULL, '/tool/gen/importTable', '127.0.0.1', '内网IP', 'dq_marking_record,dq_account_record',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 09:56:17');
INSERT INTO "sys_oper_log"
VALUES (103, '代码生成', 8, 'com.ruoyi.project.tool.gen.controller.GenController.batchGenCode()', 'GET', 1, 'admin',
        NULL, '/tool/gen/batchGenCode', '127.0.0.1', '内网IP', '{}', 'null', 0, NULL, '2020-06-20 09:57:13');
INSERT INTO "sys_oper_log"
VALUES (104, '菜单管理', 3, 'com.ruoyi.project.system.controller.SysMenuController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/menu/4', '127.0.0.1', '内网IP', '{menuId=4}', '{"msg":"菜单已分配,不允许删除","code":500}', 0,
        NULL, '2020-06-20 10:01:04');
INSERT INTO "sys_oper_log"
VALUES (105, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"guide","orderNum":"4","menuName":"会员管理","params":{},"parentId":0,"path":"/system/appUser/","children":[],"createTime":1521171180000,"isFrame":"0","menuId":4,"menuType":"M","perms":"","status":"0"}',
        '{"msg":"新增菜单''会员管理''失败，地址必须以http(s)://开头","code":500}', 0, NULL, '2020-06-20 10:01:36');
INSERT INTO "sys_oper_log"
VALUES (106, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"guide","orderNum":"4","menuName":"会员管理","params":{},"parentId":0,"path":"/system/appUser/","children":[],"createTime":1521171180000,"updateBy":"admin","isFrame":"1","menuId":4,"menuType":"M","perms":"","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:01:41');
INSERT INTO "sys_oper_log"
VALUES (107, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"guide","orderNum":"4","menuName":"会员管理","params":{},"parentId":0,"path":"appUser","children":[],"createTime":1521171180000,"updateBy":"admin","isFrame":"1","menuId":4,"menuType":"M","perms":"","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:01:54');
INSERT INTO "sys_oper_log"
VALUES (108, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"guide","orderNum":"4","menuName":"会员管理","params":{},"parentId":0,"path":"dq","children":[],"createTime":1521171180000,"updateBy":"admin","isFrame":"1","menuId":4,"menuType":"M","perms":"","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:02:25');
INSERT INTO "sys_oper_log"
VALUES (109, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"1","menuName":"会员管理","params":{},"parentId":4,"path":"appuser","createBy":"admin","children":[],"isFrame":"1","menuType":"C","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:03:52');
INSERT INTO "sys_oper_log"
VALUES (110, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"#","orderNum":"1","menuName":"会员管理","params":{},"parentId":4,"path":"appuser","component":"dq/appuser","children":[],"createTime":1592618632000,"updateBy":"admin","isFrame":"1","menuId":2000,"menuType":"C","perms":"system:appUser:appUser","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:04:37');
INSERT INTO "sys_oper_log"
VALUES (111, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"#","orderNum":"1","menuName":"会员管理","params":{},"parentId":4,"path":"appuser","component":"dq/appuser/index","children":[],"createTime":1592618632000,"updateBy":"admin","isFrame":"1","menuId":2000,"menuType":"C","perms":"system:appUser:appUser","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:04:47');
INSERT INTO "sys_oper_log"
VALUES (112, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/06/20/2a868ceb71b989e55db375729c65241c.jpeg","code":200}', 0,
        NULL, '2020-06-20 10:05:04');
INSERT INTO "sys_oper_log"
VALUES (113, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"dq_app_user_account_status","createBy":"admin","dictName":"会员状态","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:16:52');
INSERT INTO "sys_oper_log"
VALUES (114, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"0","dictSort":0,"remark":"0-未审核1-审核通过 2-审核不通过 9-停用","params":{},"dictType":"dq_app_user_account_status","dictLabel":"未审核","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:17:30');
INSERT INTO "sys_oper_log"
VALUES (115, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":0,"params":{},"dictType":"dq_app_user_account_status","dictLabel":"审核通过","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:17:46');
INSERT INTO "sys_oper_log"
VALUES (116, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"2","dictSort":0,"params":{},"dictType":"dq_app_user_account_status","dictLabel":"审核不通过","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:17:57');
INSERT INTO "sys_oper_log"
VALUES (117, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"9","dictSort":0,"params":{},"dictType":"dq_app_user_account_status","dictLabel":"停用","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:18:09');
INSERT INTO "sys_oper_log"
VALUES (118, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"dq_app_user_role_id","createBy":"admin","dictName":"会员等级","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:19:22');
INSERT INTO "sys_oper_log"
VALUES (119, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"0","dictSort":0,"params":{},"dictType":"dq_app_user_role_id","dictLabel":"普通会员","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:19:50');
INSERT INTO "sys_oper_log"
VALUES (120, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"系统会员","dictSort":0,"params":{},"dictType":"dq_app_user_role_id","dictLabel":"1","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:20:00');
INSERT INTO "sys_oper_log"
VALUES (121, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":0,"params":{},"dictType":"dq_app_user_role_id","dictLabel":"系统会员","createBy":"admin","default":false,"isDefault":"N","createTime":1592619600000,"dictCode":105,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 10:20:08');
INSERT INTO "sys_oper_log"
VALUES (122, '用户管理', 2, 'com.ruoyi.project.system.controller.SysUserController.changeStatus()', 'PUT', 1, 'admin',
        NULL, '/system/user/changeStatus', '127.0.0.1', '内网IP',
        '{"admin":false,"updateBy":"admin","params":{},"userId":2,"status":"1"}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-06-20 10:20:25');
INSERT INTO "sys_oper_log"
VALUES (123, '字典类型', 3, 'com.ruoyi.project.system.controller.SysDictDataController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/dict/data/100', '127.0.0.1', '内网IP', '{dictCodes=100}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-06-20 10:27:46');
INSERT INTO "sys_oper_log"
VALUES (124, '字典类型', 3, 'com.ruoyi.project.system.controller.SysDictDataController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/dict/data/102', '127.0.0.1', '内网IP', '{dictCodes=102}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-06-20 10:27:49');
INSERT INTO "sys_oper_log"
VALUES (125, '用户管理', 2, 'com.ruoyi.project.system.controller.SysUserController.changeStatus()', 'PUT', 1, 'admin',
        NULL, '/system/user/changeStatus', '127.0.0.1', '内网IP',
        '{"admin":false,"updateBy":"admin","params":{},"status":"0"}', '{"msg":"操作失败","code":500}', 0, NULL,
        '2020-06-20 10:43:48');
INSERT INTO "sys_oper_log"
VALUES (126, '用户管理', 2, 'com.ruoyi.project.system.controller.SysUserController.changeStatus()', 'PUT', 1, 'admin',
        NULL, '/system/user/changeStatus', '127.0.0.1', '内网IP',
        '{"admin":false,"updateBy":"admin","params":{},"status":"0"}', '{"msg":"操作失败","code":500}', 0, NULL,
        '2020-06-20 10:44:39');
INSERT INTO "sys_oper_log"
VALUES (127, '参数管理', 2, 'com.ruoyi.project.dq.controller.AppUserController.edit()', 'POST', 1, 'admin', NULL,
        '/system/appUser/edit', '127.0.0.1', '内网IP', '{"update_by":"admin"}', '{"msg":"操作失败","code":500}', 0,
        NULL, '2020-06-20 15:15:03');
INSERT INTO "sys_oper_log"
VALUES (128, '参数管理', 2, 'com.ruoyi.project.dq.controller.AppUserController.edit()', 'POST', 1, 'admin', NULL,
        '/system/appUser/edit', '127.0.0.1', '内网IP', '{"update_by":"admin"}', '{"msg":"操作失败","code":500}', 0,
        NULL, '2020-06-20 15:15:19');
INSERT INTO "sys_oper_log"
VALUES (129, '参数管理', 2, 'com.ruoyi.project.dq.controller.AppUserController.edit()', 'POST', 1, 'admin', NULL,
        '/system/appUser/edit', '127.0.0.1', '内网IP', '{"update_by":"admin"}', '{"msg":"操作失败","code":500}', 0,
        NULL, '2020-06-20 15:17:47');
INSERT INTO "sys_oper_log"
VALUES (130, '参数管理', 2, 'com.ruoyi.project.dq.controller.AppUserController.edit()', 'POST', 1, 'admin', NULL,
        '/system/appUser/edit', '127.0.0.1', '内网IP', '{"update_by":"admin"}', '{"msg":"操作失败","code":500}', 0,
        NULL, '2020-06-20 15:18:33');
INSERT INTO "sys_oper_log"
VALUES (131, '参数管理', 2, 'com.ruoyi.project.dq.controller.AppUserController.edit()', 'POST', 1, 'admin', NULL,
        '/system/appUser/edit', '127.0.0.1', '内网IP',
        '{"app_user_id":45,"username":"18073721772","account_status":"0","update_by":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:20:25');
INSERT INTO "sys_oper_log"
VALUES (132, '参数管理', 2, 'com.ruoyi.project.dq.controller.AppUserController.edit()', 'POST', 1, 'admin', NULL,
        '/system/appUser/edit', '127.0.0.1', '内网IP',
        '{"app_user_id":14,"username":"15979081211","password":"1e8a084f6202ebc080369c34ddc4feb6","update_by":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:27:39');
INSERT INTO "sys_oper_log"
VALUES (133, '参数管理', 3, 'com.ruoyi.project.dq.controller.AppUserController.remove()', 'DELETE', 1, 'admin', NULL,
        '/system/appUser/31', '127.0.0.1', '内网IP', '{userIds=31}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-06-20 15:31:09');
INSERT INTO "sys_oper_log"
VALUES (134, '参数管理', 3, 'com.ruoyi.project.dq.controller.AppUserController.remove()', 'DELETE', 1, 'admin', NULL,
        '/system/appUser/14', '127.0.0.1', '内网IP', '{userIds=14}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-06-20 15:31:38');
INSERT INTO "sys_oper_log"
VALUES (135, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"2","menuName":"充值管理","params":{},"parentId":4,"path":"recharge","createBy":"admin","children":[],"isFrame":"1","menuType":"M","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:33:46');
INSERT INTO "sys_oper_log"
VALUES (136, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"#","orderNum":"2","menuName":"充值管理","params":{},"parentId":4,"path":"recharge","component":"dq/recharge/index","children":[],"createTime":1592638426000,"updateBy":"admin","isFrame":"1","menuId":2001,"menuType":"C","perms":"system:recharge:recharge","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:34:53');
INSERT INTO "sys_oper_log"
VALUES (137, '代码生成', 6, 'com.ruoyi.project.tool.gen.controller.GenController.importTableSave()', 'POST', 1, 'admin',
        NULL, '/tool/gen/importTable', '127.0.0.1', '内网IP', 'dq_pay_order', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-06-20 15:44:43');
INSERT INTO "sys_oper_log"
VALUES (138, '代码生成', 8, 'com.ruoyi.project.tool.gen.controller.GenController.batchGenCode()', 'GET', 1, 'admin',
        NULL, '/tool/gen/batchGenCode', '127.0.0.1', '内网IP', '{}', 'null', 0, NULL, '2020-06-20 15:44:51');
INSERT INTO "sys_oper_log"
VALUES (139, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"dq_pay_order_pay_platform","createBy":"admin","dictName":"充值方式","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:46:07');
INSERT INTO "sys_oper_log"
VALUES (140, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"online","dictSort":0,"params":{},"dictType":"dq_pay_order_pay_platform","dictLabel":"手动充值","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:46:36');
INSERT INTO "sys_oper_log"
VALUES (141, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"alipay","dictSort":1,"params":{},"dictType":"dq_pay_order_pay_platform","dictLabel":"支付宝","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:46:56');
INSERT INTO "sys_oper_log"
VALUES (142, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"wxpay","dictSort":3,"params":{},"dictType":"dq_pay_order_pay_platform","dictLabel":"微信充值","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 15:47:13');
INSERT INTO "sys_oper_log"
VALUES (143, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"dq_pay_order_pay_type","createBy":"admin","dictName":"支付类型","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:01:41');
INSERT INTO "sys_oper_log"
VALUES (144, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"10","dictSort":0,"remark":"支付类型(10=在线支付、20=线下支付)","params":{},"dictType":"dq_pay_order_pay_type","dictLabel":"在线支付","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:02:41');
INSERT INTO "sys_oper_log"
VALUES (145, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"20","dictSort":1,"remark":"支付类型(10=在线支付、20=线下支付)","params":{},"dictType":"dq_pay_order_pay_type","dictLabel":"线下支付","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:03:06');
INSERT INTO "sys_oper_log"
VALUES (146, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"remark":"支付状态(10=待支付、20=支付失败、30=支付成功、40=已关闭、50=已取消)","params":{},"dictType":"dq_pay_order_pay_status","createBy":"admin","dictName":"支付状态","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:03:57');
INSERT INTO "sys_oper_log"
VALUES (147, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"10","dictSort":1,"remark":"支付状态(10=待支付、20=支付失败、30=支付成功、40=已关闭、50=已取消)","params":{},"dictType":"dq_pay_order_pay_status","dictLabel":"待支付","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:04:30');
INSERT INTO "sys_oper_log"
VALUES (148, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"20","dictSort":2,"params":{},"dictType":"dq_pay_order_pay_status","dictLabel":"支付失败","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:04:39');
INSERT INTO "sys_oper_log"
VALUES (149, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"30","dictSort":3,"params":{},"dictType":"dq_pay_order_pay_status","dictLabel":"支付成功","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:04:49');
INSERT INTO "sys_oper_log"
VALUES (150, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"40","dictSort":4,"params":{},"dictType":"dq_pay_order_pay_status","dictLabel":"已关闭","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:04:57');
INSERT INTO "sys_oper_log"
VALUES (151, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"50","dictSort":5,"params":{},"dictType":"dq_pay_order_pay_status","dictLabel":"已取消","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:05:07');
INSERT INTO "sys_oper_log"
VALUES (152, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"dq_pay_order_recharge_type","createBy":"admin","dictName":"充值类型","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:07:13');
INSERT INTO "sys_oper_log"
VALUES (153, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"0","dictSort":0,"params":{},"dictType":"dq_pay_order_recharge_type","dictLabel":"老的","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:07:27');
INSERT INTO "sys_oper_log"
VALUES (154, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":0,"params":{},"dictType":"dq_pay_order_recharge_type","dictLabel":"对冲","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:07:33');
INSERT INTO "sys_oper_log"
VALUES (155, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"2","dictSort":0,"params":{},"dictType":"dq_pay_order_recharge_type","dictLabel":"漏充","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:07:44');
INSERT INTO "sys_oper_log"
VALUES (156, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"3","dictSort":0,"params":{},"dictType":"dq_pay_order_recharge_type","dictLabel":"补偿亏损","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-20 16:07:53');
INSERT INTO "sys_oper_log"
VALUES (157, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"5","menuName":"打标管理","params":{},"parentId":0,"path":"dq","createBy":"admin","children":[],"isFrame":"1","menuType":"M","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 10:31:10');
INSERT INTO "sys_oper_log"
VALUES (158, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"1","menuName":"打标配置","params":{},"parentId":2002,"path":"dqconfig","component":"dq/config/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:dqconfig:config","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 10:32:08');
INSERT INTO "sys_oper_log"
VALUES (159, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"#","orderNum":"5","menuName":"打标管理","params":{},"parentId":0,"path":"dqmarking","children":[],"createTime":1592706670000,"updateBy":"admin","isFrame":"1","menuId":2002,"menuType":"M","perms":"","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 10:34:05');
INSERT INTO "sys_oper_log"
VALUES (160, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"2","menuName":"打标记录","params":{},"parentId":2002,"path":"markingrecord","component":"dq/marking/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:markingRecord:markingRecord","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 11:41:12');
INSERT INTO "sys_oper_log"
VALUES (161, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"6","menuName":"统计管理","params":{},"path":"stastitac","createBy":"admin","children":[],"isFrame":"1","menuType":"M","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:15:27');
INSERT INTO "sys_oper_log"
VALUES (162, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"1","menuName":"验号统计","params":{},"parentId":2005,"path":"yhstatistac","component":"dq/statistics/yhindex","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:dsjfstatistics:yhlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:16:20');
INSERT INTO "sys_oper_log"
VALUES (163, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"2","menuName":"流量统计","params":{},"parentId":2005,"path":"flowstatistics","component":"da/statistics/flowindex","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:dsjfstatistics:flowlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:17:09');
INSERT INTO "sys_oper_log"
VALUES (164, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"#","orderNum":"1","menuName":"验号统计","params":{},"parentId":2005,"path":"yhstatistics","component":"dq/statistics/yhindex","children":[],"createTime":1592723780000,"updateBy":"admin","isFrame":"1","menuId":2006,"menuType":"C","perms":"system:dsjfstatistics:yhlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:19:59');
INSERT INTO "sys_oper_log"
VALUES (165, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"build","orderNum":"5","menuName":"打标管理","params":{},"parentId":0,"path":"dqmarking","children":[],"createTime":1592706670000,"updateBy":"admin","isFrame":"1","menuId":2002,"menuType":"M","perms":"","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:22:31');
INSERT INTO "sys_oper_log"
VALUES (166, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"code","orderNum":"6","menuName":"统计管理","params":{},"parentId":0,"path":"stastitac","children":[],"createTime":1592723727000,"updateBy":"admin","isFrame":"1","menuId":2005,"menuType":"M","perms":"","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:22:41');
INSERT INTO "sys_oper_log"
VALUES (167, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"#","orderNum":"2","menuName":"流量统计","params":{},"parentId":2005,"path":"flowstatistics","component":"dq/statistics/flowindex","children":[],"createTime":1592723829000,"updateBy":"admin","isFrame":"1","menuId":2007,"menuType":"C","perms":"system:dsjfstatistics:flowlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:23:11');
INSERT INTO "sys_oper_log"
VALUES (168, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updatePwd()', 'PUT', 1, 'admin',
        NULL, '/system/user/profile/updatePwd', '127.0.0.1', '内网IP', 'admin123 111111',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-21 15:43:32');
INSERT INTO "sys_oper_log"
VALUES (169, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"dq_pdt_flow_task_type","createBy":"admin","dictName":"派大淘流量大类","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 17:54:25');
INSERT INTO "sys_oper_log"
VALUES (170, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"48","dictSort":1,"params":{},"dictType":"dq_pdt_flow_task_type","dictLabel":"精准流量","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 17:57:27');
INSERT INTO "sys_oper_log"
VALUES (171, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":1,"params":{},"dictType":"dq_pdt_flow_task_type","dictLabel":"流量任务","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 17:58:45');
INSERT INTO "sys_oper_log"
VALUES (172, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"2","dictSort":1,"params":{},"dictType":"dq_pdt_flow_task_type","dictLabel":"收藏任务","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 17:59:09');
INSERT INTO "sys_oper_log"
VALUES (173, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"3","dictSort":1,"params":{},"dictType":"dq_pdt_flow_task_type","dictLabel":"加购任务","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 17:59:30');
INSERT INTO "sys_oper_log"
VALUES (174, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"106","dictSort":1,"params":{},"dictType":"dq_pdt_flow_task_type","dictLabel":"商品推荐","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 17:59:56');
INSERT INTO "sys_oper_log"
VALUES (175, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"60","dictSort":1,"params":{},"dictType":"dq_pdt_flow_task_type","dictLabel":"关注店铺","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 18:00:11');
INSERT INTO "sys_oper_log"
VALUES (176, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"101","dictSort":1,"params":{},"dictType":"dq_pdt_flow_task_type","dictLabel":"开团提醒","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 18:00:26');
INSERT INTO "sys_oper_log"
VALUES (177, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"浏览任务商品时间","createBy":"admin","dictName":"bq_browse_goods_time_id","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:35:44');
INSERT INTO "sys_oper_log"
VALUES (178, '字典类型', 2, 'com.ruoyi.project.system.controller.SysDictTypeController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"dictId":107,"params":{},"dictType":"bq_browse_goods_time_id","createBy":"admin","createTime":1592912144000,"updateBy":"admin","dictName":"浏览任务商品时间","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:36:44');
INSERT INTO "sys_oper_log"
VALUES (179, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"browse_other_goods","createBy":"admin","dictName":"深度浏览其他商品","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:38:41');
INSERT INTO "sys_oper_log"
VALUES (180, '代码生成', 6, 'com.ruoyi.project.tool.gen.controller.GenController.importTableSave()', 'POST', 1, 'admin',
        NULL, '/tool/gen/importTable', '127.0.0.1', '内网IP', 'dq_pdt_flow', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-06-23 19:40:01');
INSERT INTO "sys_oper_log"
VALUES (181, '字典类型', 2, 'com.ruoyi.project.system.controller.SysDictTypeController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"dictId":108,"params":{},"dictType":"dq_browse_other_goods","createBy":"admin","createTime":1592912321000,"updateBy":"admin","dictName":"深度浏览其他商品","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:40:05');
INSERT INTO "sys_oper_log"
VALUES (182, '代码生成', 8, 'com.ruoyi.project.tool.gen.controller.GenController.batchGenCode()', 'GET', 1, 'admin',
        NULL, '/tool/gen/batchGenCode', '127.0.0.1', '内网IP', '{}', 'null', 0, NULL, '2020-06-23 19:40:06');
INSERT INTO "sys_oper_log"
VALUES (183, '字典类型', 2, 'com.ruoyi.project.system.controller.SysDictTypeController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"dictId":107,"params":{},"dictType":"dq_browse_goods_time_id","createBy":"admin","createTime":1592912144000,"updateBy":"admin","dictName":"浏览任务商品时间","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:40:14');
INSERT INTO "sys_oper_log"
VALUES (184, '字典类型', 2, 'com.ruoyi.project.system.controller.SysDictTypeController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"remark":"浏览任务商品时间id,不传默认为1,","dictId":107,"params":{},"dictType":"dq_browse_goods_time_id","createBy":"admin","createTime":1592912144000,"updateBy":"admin","dictName":"浏览任务商品时间","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:41:07');
INSERT INTO "sys_oper_log"
VALUES (185, '字典类型', 2, 'com.ruoyi.project.system.controller.SysDictTypeController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"remark":"需要深度浏览其他商品时,不传默认为1,","dictId":108,"params":{},"dictType":"dq_browse_other_goods","createBy":"admin","createTime":1592912321000,"updateBy":"admin","dictName":"深度浏览其他商品","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:41:24');
INSERT INTO "sys_oper_log"
VALUES (186, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":0,"remark":"0","params":{},"dictType":"dq_browse_other_goods","dictLabel":"浏览121-180秒（免费）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:45:04');
INSERT INTO "sys_oper_log"
VALUES (187, '字典类型', 3, 'com.ruoyi.project.system.controller.SysDictDataController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/dict/data/127', '127.0.0.1', '内网IP', '{dictCodes=127}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-06-23 19:45:14');
INSERT INTO "sys_oper_log"
VALUES (188, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":0,"remark":"0","params":{},"dictType":"dq_browse_goods_time_id","dictLabel":"浏览121-180秒（免费）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:45:34');
INSERT INTO "sys_oper_log"
VALUES (189, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"2","dictSort":0,"remark":"5","params":{},"dictType":"dq_browse_goods_time_id","dictLabel":"浏览181-300秒（ 5金币）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:46:32');
INSERT INTO "sys_oper_log"
VALUES (190, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"3","dictSort":0,"remark":"10","params":{},"dictType":"dq_browse_goods_time_id","dictLabel":"浏览181-300秒（ 5金币）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:47:23');
INSERT INTO "sys_oper_log"
VALUES (191, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"3","dictSort":0,"remark":"10","params":{},"dictType":"dq_browse_goods_time_id","dictLabel":"浏览301-420秒（ 10金币）","createBy":"admin","default":false,"isDefault":"N","createTime":1592912843000,"dictCode":130,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:48:11');
INSERT INTO "sys_oper_log"
VALUES (192, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":0,"remark":"0","params":{},"dictType":"dq_browse_other_goods","dictLabel":"深入1个商品（免费）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:48:59');
INSERT INTO "sys_oper_log"
VALUES (193, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"2","dictSort":0,"remark":"\t2","params":{},"dictType":"dq_browse_other_goods","dictLabel":"随机深入浏览1-2个商品（+2金币)","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:49:31');
INSERT INTO "sys_oper_log"
VALUES (194, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"3","dictSort":0,"remark":"3","params":{},"dictType":"dq_browse_other_goods","dictLabel":"随机深入浏览1-3个商品（+3金币）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:49:41');
INSERT INTO "sys_oper_log"
VALUES (195, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"5","dictSort":0,"remark":"3","params":{},"dictType":"dq_browse_other_goods","dictLabel":"深入浏览2个商品（+3金币）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:49:59');
INSERT INTO "sys_oper_log"
VALUES (196, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"6","dictSort":0,"remark":"4","params":{},"dictType":"dq_browse_other_goods","dictLabel":"深入浏览3个商品（+4金币）","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:14');
INSERT INTO "sys_oper_log"
VALUES (197, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"7","dictSort":0,"remark":"0","params":{},"dictType":"dq_browse_other_goods","dictLabel":"不深入浏览","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:30');
INSERT INTO "sys_oper_log"
VALUES (198, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"6","dictSort":1,"remark":"4","params":{},"dictType":"dq_browse_other_goods","dictLabel":"深入浏览3个商品（+4金币）","createBy":"admin","default":false,"isDefault":"N","createTime":1592913014000,"dictCode":135,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:34');
INSERT INTO "sys_oper_log"
VALUES (199, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"6","dictSort":1,"remark":"4","params":{},"dictType":"dq_browse_other_goods","dictLabel":"深入浏览3个商品（+4金币）","createBy":"admin","default":false,"isDefault":"N","createTime":1592913014000,"dictCode":135,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:37');
INSERT INTO "sys_oper_log"
VALUES (200, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"7","dictSort":1,"remark":"0","params":{},"dictType":"dq_browse_other_goods","dictLabel":"不深入浏览","createBy":"admin","default":false,"isDefault":"N","createTime":1592913030000,"dictCode":136,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:40');
INSERT INTO "sys_oper_log"
VALUES (201, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"5","dictSort":1,"remark":"3","params":{},"dictType":"dq_browse_other_goods","dictLabel":"深入浏览2个商品（+3金币）","createBy":"admin","default":false,"isDefault":"N","createTime":1592912999000,"dictCode":134,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:43');
INSERT INTO "sys_oper_log"
VALUES (202, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"3","dictSort":1,"remark":"3","params":{},"dictType":"dq_browse_other_goods","dictLabel":"随机深入浏览1-3个商品（+3金币）","createBy":"admin","default":false,"isDefault":"N","createTime":1592912981000,"dictCode":133,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:54');
INSERT INTO "sys_oper_log"
VALUES (203, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"2","dictSort":1,"remark":"\t2","params":{},"dictType":"dq_browse_other_goods","dictLabel":"随机深入浏览1-2个商品（+2金币)","createBy":"admin","default":false,"isDefault":"N","createTime":1592912971000,"dictCode":132,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:50:57');
INSERT INTO "sys_oper_log"
VALUES (204, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":1,"remark":"0","params":{},"dictType":"dq_browse_other_goods","dictLabel":"深入1个商品（免费）","createBy":"admin","default":false,"isDefault":"N","createTime":1592912939000,"dictCode":131,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:51:04');
INSERT INTO "sys_oper_log"
VALUES (205, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"1","dictSort":1,"remark":"0","params":{},"dictType":"dq_browse_goods_time_id","dictLabel":"浏览121-180秒（免费）","createBy":"admin","default":false,"isDefault":"N","createTime":1592912734000,"dictCode":128,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:51:32');
INSERT INTO "sys_oper_log"
VALUES (206, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"3","dictSort":1,"remark":"10","params":{},"dictType":"dq_browse_goods_time_id","dictLabel":"浏览301-420秒（ 10金币）","createBy":"admin","default":false,"isDefault":"N","createTime":1592912843000,"dictCode":130,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:51:35');
INSERT INTO "sys_oper_log"
VALUES (207, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"2","dictSort":1,"remark":"5","params":{},"dictType":"dq_browse_goods_time_id","dictLabel":"浏览181-300秒（ 5金币）","createBy":"admin","default":false,"isDefault":"N","createTime":1592912792000,"dictCode":129,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 19:51:42');
INSERT INTO "sys_oper_log"
VALUES (208, '字典类型', 1, 'com.ruoyi.project.system.controller.SysDictTypeController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/type', '127.0.0.1', '内网IP',
        '{"params":{},"dictType":"dq_pdt_flow_task_sub_type","createBy":"admin","dictName":"派大淘流量子类","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-23 20:17:25');
INSERT INTO "sys_oper_log"
VALUES (209, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"3","menuName":"派大淘流量统计","params":{},"parentId":2005,"path":"pdtstatistics","component":"dq/statistics/pdtflowindex","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:pdtstatistics:flowlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-30 17:25:36');
INSERT INTO "sys_oper_log"
VALUES (210, '代码生成', 6, 'com.ruoyi.project.tool.gen.controller.GenController.importTableSave()', 'POST', 1, 'admin',
        NULL, '/tool/gen/importTable', '127.0.0.1', '内网IP', 'dq_call_record', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-06-30 20:04:59');
INSERT INTO "sys_oper_log"
VALUES (211, '代码生成', 2, 'com.ruoyi.project.tool.gen.controller.GenController.editSave()', 'PUT', 1, 'admin', NULL,
        '/tool/gen', '127.0.0.1', '内网IP',
        '{"functionAuthor":"lur","columns":[{"usableColumn":false,"columnId":73,"isIncrement":"0","increment":false,"insert":true,"dictType":"","required":false,"superColumn":false,"updateBy":"","isInsert":"1","javaField":"callId","htmlType":"input","edit":false,"query":false,"columnComment":"主键id","sort":1,"list":false,"params":{},"javaType":"Long","queryType":"EQ","columnType":"bigint(20)","createBy":"admin","isPk":"1","createTime":1593518698000,"tableId":5,"pk":true,"columnName":"call_id"},{"usableColumn":false,"columnId":74,"isIncrement":"0","increment":false,"insert":true,"isList":"1","dictType":"","required":false,"superColumn":false,"updateBy":"","isInsert":"1","javaField":"interfaceAddr","htmlType":"input","edit":true,"query":true,"columnComment":"接口地址","isQuery":"1","sort":2,"list":true,"params":{},"javaType":"String","queryType":"EQ","columnType":"varchar(255)","createBy":"admin","isPk":"0","createTime":1593518698000,"isEdit":"1","tableId":5,"pk":false,"columnName":"interface_addr"},{"usableColumn":false,"columnId":75,"isIncrement":"0","increment":false,"insert":true,"isList":"1","dictType":"","required":false,"superColumn":false,"updateBy":"","isInsert":"1","javaField":"interfaceParams","htmlType":"textarea","edit":true,"query":true,"columnComment":"接口参数","isQuery":"1","sort":3,"list":true,"params":{},"javaType":"String","queryType":"EQ","columnType":"varchar(1000)","createBy":"admin","isPk":"0","createTime":1593518698000,"isEdit":"1","tableId":5,"pk":false,"columnName":"interface_params"},{"usableColumn":false,"columnId":76,"isIncrement":"0","increment":false,"insert":true,"isList":"1","dictType":"","required":false,"superColumn":false,"updateBy":"","isInsert":"1","javaField":"callTime","htmlType":"datetime","edit":true,"query":true,"columnComment":"调用时间","isQuery":"1","sort":4,"list":true,"params":{},"javaType":"Date","queryType":"EQ","columnType":"datetime","createBy":"admin","isPk":"0","createTime":1593518698000,"isEdit":"1","tableId":5,"pk":false,"columnName"',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-30 20:05:43');
INSERT INTO "sys_oper_log"
VALUES (212, '代码生成', 8, 'com.ruoyi.project.tool.gen.controller.GenController.batchGenCode()', 'GET', 1, 'admin',
        NULL, '/tool/gen/batchGenCode', '127.0.0.1', '内网IP', '{}', 'null', 0, NULL, '2020-06-30 20:06:53');
INSERT INTO "sys_oper_log"
VALUES (213, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"3","menuName":"接口管理","params":{},"parentId":4,"path":"callrecord","component":"dq/callrecord/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:callrecord:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-06-30 20:54:30');
INSERT INTO "sys_oper_log"
VALUES (214, '定时任务', 1, 'com.ruoyi.project.monitor.controller.SysJobController.add()', 'POST', 1, 'admin', NULL,
        '/monitor/job', '192.168.0.166', '内网IP',
        '{"jobName":"派大淘流量任务","concurrent":"1","jobGroup":"DEFAULT","params":{},"cronExpression":"0 */10 * * * ?","jobId":100,"nextValidTime":1593679200000,"invokeTarget":"dqTask.checkPdtFlowData","misfirePolicy":"1","status":"1"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:30:46');
INSERT INTO "sys_oper_log"
VALUES (215, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '192.168.0.166', '内网IP',
        '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-02 16:32:15');
INSERT INTO "sys_oper_log"
VALUES (216, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.changeStatus()', 'PUT', 1, 'admin',
        NULL, '/monitor/job/changeStatus', '192.168.0.166', '内网IP',
        '{"params":{},"jobId":100,"misfirePolicy":"0","status":"0"}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-02 16:32:19');
INSERT INTO "sys_oper_log"
VALUES (217, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '192.168.0.166', '内网IP',
        '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-02 16:39:02');
INSERT INTO "sys_oper_log"
VALUES (218, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:42:27');
INSERT INTO "sys_oper_log"
VALUES (219, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.edit()', 'PUT', 1, 'admin', NULL,
        '/monitor/job', '192.168.0.166', '内网IP',
        '{"jobName":"派大淘流量任务","concurrent":"0","remark":"","jobGroup":"DEFAULT","params":{},"cronExpression":"0 */10 * * * ?","jobId":100,"createBy":"","nextValidTime":1593679800000,"createTime":1593678646000,"invokeTarget":"dqTask.checkPdtFlowData","misfirePolicy":"1","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:43:20');
INSERT INTO "sys_oper_log"
VALUES (220, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '192.168.0.166', '内网IP',
        '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-02 16:43:23');
INSERT INTO "sys_oper_log"
VALUES (221, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:44:17');
INSERT INTO "sys_oper_log"
VALUES (222, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:51:42');
INSERT INTO "sys_oper_log"
VALUES (223, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:51:52');
INSERT INTO "sys_oper_log"
VALUES (224, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:56:37');
INSERT INTO "sys_oper_log"
VALUES (225, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:57:07');
INSERT INTO "sys_oper_log"
VALUES (226, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:57:54');
INSERT INTO "sys_oper_log"
VALUES (227, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 16:58:00');
INSERT INTO "sys_oper_log"
VALUES (228, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 17:05:56');
INSERT INTO "sys_oper_log"
VALUES (229, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 17:06:31');
INSERT INTO "sys_oper_log"
VALUES (230, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '192.168.0.166', '内网IP',
        '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-02 17:08:41');
INSERT INTO "sys_oper_log"
VALUES (231, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"1","menuName":"门户网站在线用户","params":{},"parentId":2,"path":"webonline","component":"monitor/webonline/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"monitor:webonline:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 17:29:48');
INSERT INTO "sys_oper_log"
VALUES (232, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","icon":"user","orderNum":"1","menuName":"门户网站在线用户","params":{},"parentId":2,"path":"webonline","component":"monitor/webonline/index","children":[],"createTime":1593682187000,"updateBy":"admin","isFrame":"1","menuId":2010,"menuType":"C","perms":"monitor:webonline:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 17:30:48');
INSERT INTO "sys_oper_log"
VALUES (233, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.run()', 'PUT', 1, 'admin', NULL,
        '/monitor/job/run', '127.0.0.1', '内网IP', '{"jobGroup":"DEFAULT","params":{},"jobId":100,"misfirePolicy":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-02 20:23:59');
INSERT INTO "sys_oper_log"
VALUES (234, '通知公告', 1, 'com.ruoyi.project.system.controller.SysNoticeController.add()', 'POST', 1, 'admin', NULL,
        '/system/notice', '192.168.0.166', '内网IP',
        '{"noticeType":"1","params":{},"noticeTitle":"双11加购通知","noticeContent":"<p>双11加购通知</p>","createBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-06 15:43:58');
INSERT INTO "sys_oper_log"
VALUES (235, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"4","menuName":"打标统计","params":{},"parentId":2005,"path":"makinglist","component":"dq/statistics/dbindex","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:dsjfstatistics:makinglist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-08 10:48:03');
INSERT INTO "sys_oper_log"
VALUES (236, '菜单管理', 3, 'com.ruoyi.project.system.controller.SysMenuController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/menu/2004', '127.0.0.1', '内网IP', '{menuId=2004}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-08 11:07:08');
INSERT INTO "sys_oper_log"
VALUES (237, '菜单管理', 3, 'com.ruoyi.project.system.controller.SysMenuController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/menu/2003', '127.0.0.1', '内网IP', '{menuId=2003}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-08 11:07:14');
INSERT INTO "sys_oper_log"
VALUES (238, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"1","menuName":"打标记录","params":{},"parentId":2002,"path":"markingRecord","component":"dq/marking/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:markingRecord:markingRecord","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-08 11:08:27');
INSERT INTO "sys_oper_log"
VALUES (239, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"2","menuName":"验号记录","params":{},"parentId":2002,"path":"yhrecord","component":"dq/yhrecord/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:yhrecord:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-08 11:35:59');
INSERT INTO "sys_oper_log"
VALUES (240, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"3","menuName":"电商教父流量记录","params":{},"parentId":2002,"path":"flowrecord","component":"dq/flowrecord/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:flowrecord:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-08 11:36:46');
INSERT INTO "sys_oper_log"
VALUES (241, '菜单管理', 1, 'com.ruoyi.project.system.controller.SysMenuController.add()', 'POST', 1, 'admin', NULL,
        '/system/menu', '127.0.0.1', '内网IP',
        '{"visible":"0","orderNum":"4","menuName":"派大淘流量记录","params":{},"parentId":2002,"path":"pdtflow","component":"dq/pdtflow/index","createBy":"admin","children":[],"isFrame":"1","menuType":"C","perms":"system:pdtflow:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-08 11:37:44');
INSERT INTO "sys_oper_log"
VALUES (242, '字典数据', 1, 'com.ruoyi.project.system.controller.SysDictDataController.add()', 'POST', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"award","dictSort":5,"params":{},"dictType":"dq_pay_order_pay_platform","dictLabel":"平台奖励","createBy":"admin","default":false,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-10 17:25:40');
INSERT INTO "sys_oper_log"
VALUES (243, '字典数据', 2, 'com.ruoyi.project.system.controller.SysDictDataController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/dict/data', '127.0.0.1', '内网IP',
        '{"dictValue":"award","dictSort":4,"params":{},"dictType":"dq_pay_order_pay_platform","dictLabel":"平台奖励","createBy":"admin","default":false,"isDefault":"N","createTime":1594373138000,"dictCode":137,"updateBy":"admin","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-10 17:25:47');
INSERT INTO "sys_oper_log"
VALUES (244, '定时任务', 1, 'com.ruoyi.project.monitor.controller.SysJobController.add()', 'POST', 1, 'admin', NULL,
        '/monitor/job', '192.168.0.128', '内网IP',
        '{"jobName":"获取验号结果","concurrent":"1","jobGroup":"SYSTEM","params":{},"cronExpression":"*/10 * * * * ?","jobId":101,"nextValidTime":1595237740000,"invokeTarget":"dqTask.checkYhData","misfirePolicy":"1","status":"1"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-20 17:35:39');
INSERT INTO "sys_oper_log"
VALUES (245, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.changeStatus()', 'PUT', 1, 'admin',
        NULL, '/monitor/job/changeStatus', '192.168.0.128', '内网IP',
        '{"params":{},"jobId":101,"misfirePolicy":"0","status":"0"}', '{"msg":"操作成功","code":200}', 0, NULL,
        '2020-07-20 17:35:45');
INSERT INTO "sys_oper_log"
VALUES (246, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"验号消费金币","configType":"Y","configValue":"3","params":{},"configKey":"single_bid_yh_price","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:34:46');
INSERT INTO "sys_oper_log"
VALUES (247, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"验号成本","configType":"Y","configValue":"0.03","params":{},"configKey":"single_cost_yh_price","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:35:09');
INSERT INTO "sys_oper_log"
VALUES (248, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标延时时间","configType":"Y","configValue":"30 * 1000","params":{},"configKey":"marking_delay_time","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:41:07');
INSERT INTO "sys_oper_log"
VALUES (249, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标允许失败次数","configType":"Y","configValue":"3","params":{},"configKey":"marking_failed_continue_num","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:41:37');
INSERT INTO "sys_oper_log"
VALUES (250, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标日查询次数","configType":"Y","configValue":"3","params":{},"configKey":"query_marking_day_num","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:46:06');
INSERT INTO "sys_oper_log"
VALUES (251, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标授权","configType":"Y","configValue":"4956de596a564ee3be14604d0a0e4e9e","params":{},"configKey":"marking_token","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:55:20');
INSERT INTO "sys_oper_log"
VALUES (252, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标地址","configType":"Y","configValue":"http://dianshangjiaofu.com/api/dabiao/push/alone.html","params":{},"configKey":"marking_url","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:55:47');
INSERT INTO "sys_oper_log"
VALUES (253, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标地址","configType":"Y","configValue":"http://dianshangjiaofu.com/api/dabiao/push/alone.html","params":{},"configKey":"dsjy_marking_url","createBy":"admin","createTime":1595300147000,"updateBy":"admin","configId":106}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:56:54');
INSERT INTO "sys_oper_log"
VALUES (254, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标授权","configType":"Y","configValue":"4956de596a564ee3be14604d0a0e4e9e","params":{},"configKey":"dsjy_token","createBy":"admin","createTime":1595300120000,"updateBy":"admin","configId":105}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 10:57:06');
INSERT INTO "sys_oper_log"
VALUES (255, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '192.168.0.167', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/2fb6fe782752ca01351a61d6f42f2334.jpeg","code":200}', 0,
        NULL, '2020-07-21 12:00:23');
INSERT INTO "sys_oper_log"
VALUES (256, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '192.168.0.167', '内网IP',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15888888888","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"若依","sex":"1","deptId":103,"avatar":"/profile/avatar/2020/06/20/2a868ceb71b989e55db375729c65241c.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 12:00:24');
INSERT INTO "sys_oper_log"
VALUES (257, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '192.168.0.167', '内网IP',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15888888888","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"管理员","sex":"1","deptId":103,"avatar":"/profile/avatar/2020/06/20/2a868ceb71b989e55db375729c65241c.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 12:00:37');
INSERT INTO "sys_oper_log"
VALUES (258, '参数管理', 3, 'com.ruoyi.project.system.controller.SysConfigController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/config/106', '192.168.0.167', '内网IP', '{configIds=106}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-07-21 14:05:24');
INSERT INTO "sys_oper_log"
VALUES (259, '参数管理', 3, 'com.ruoyi.project.system.controller.SysConfigController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/config/104', '192.168.0.167', '内网IP', '{configIds=104}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-07-21 14:06:55');
INSERT INTO "sys_oper_log"
VALUES (260, '参数管理', 3, 'com.ruoyi.project.system.controller.SysConfigController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/config/103', '192.168.0.167', '内网IP', '{configIds=103}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-07-21 14:06:58');
INSERT INTO "sys_oper_log"
VALUES (261, '参数管理', 3, 'com.ruoyi.project.system.controller.SysConfigController.remove()', 'DELETE', 1, 'admin',
        NULL, '/system/config/102', '192.168.0.167', '内网IP', '{configIds=102}', '{"msg":"操作成功","code":200}', 0,
        NULL, '2020-07-21 14:07:01');
INSERT INTO "sys_oper_log"
VALUES (262, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"单次打标成本","remark":"单位金币","configType":"Y","configValue":"3","params":{},"configKey":"single_bid_marking_price","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:07:45');
INSERT INTO "sys_oper_log"
VALUES (263, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"验号单价","remark":"金币","configType":"Y","configValue":"3","params":{},"configKey":"single_bid_yh_price","createBy":"admin","createTime":1595298886000,"updateBy":"admin","configId":100}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:08:06');
INSERT INTO "sys_oper_log"
VALUES (264, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"验号成本","remark":"元","configType":"Y","configValue":"0.03","params":{},"configKey":"single_cost_yh_price","createBy":"admin","createTime":1595298909000,"updateBy":"admin","configId":101}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:08:14');
INSERT INTO "sys_oper_log"
VALUES (265, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标单价","remark":"单位金币","configType":"Y","configValue":"3","params":{},"configKey":"single_bid_marking_price","createBy":"admin","createTime":1595311665000,"updateBy":"admin","configId":107}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:08:23');
INSERT INTO "sys_oper_log"
VALUES (266, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标成本","remark":"单位元","configType":"Y","configValue":"0.06","params":{},"configKey":"single_cost_marking_price","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:08:45');
INSERT INTO "sys_oper_log"
VALUES (267, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标单价","remark":"金币","updateTime":1595311703000,"configType":"Y","configValue":"3","params":{},"configKey":"single_bid_marking_price","createBy":"admin","createTime":1595311665000,"updateBy":"admin","configId":107}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:08:52');
INSERT INTO "sys_oper_log"
VALUES (268, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标成本","remark":"元","configType":"Y","configValue":"0.06","params":{},"configKey":"single_cost_marking_price","createBy":"admin","createTime":1595311725000,"updateBy":"admin","configId":108}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:08:57');
INSERT INTO "sys_oper_log"
VALUES (269, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘流量单价","remark":"金币","configType":"Y","configValue":"3","params":{},"configKey":"single_bid_pdtflow_price","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:11:05');
INSERT INTO "sys_oper_log"
VALUES (270, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘流量成本","remark":"元","configType":"Y","configValue":"0.04","params":{},"configKey":"single_cost_pdtflow_price","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:12:05');
INSERT INTO "sys_oper_log"
VALUES (271, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘APPID","configType":"Y","configValue":"11217778","params":{},"configKey":"pdt_appid","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:14:47');
INSERT INTO "sys_oper_log"
VALUES (272, '参数管理', 1, 'com.ruoyi.project.system.controller.SysConfigController.add()', 'POST', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘密钥","configType":"Y","configValue":"02479ea0b17e9cfbe5513dd8f9503dd4","params":{},"configKey":"pdt_appsecrect","createBy":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:15:23');
INSERT INTO "sys_oper_log"
VALUES (273, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"验号单价","remark":"金币","updateTime":1595311686000,"configType":"N","configValue":"3","params":{},"configKey":"single_bid_yh_price","createBy":"admin","createTime":1595298886000,"updateBy":"admin","configId":100}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:15:47');
INSERT INTO "sys_oper_log"
VALUES (274, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"验号成本","remark":"元","updateTime":1595311694000,"configType":"N","configValue":"0.03","params":{},"configKey":"single_cost_yh_price","createBy":"admin","createTime":1595298909000,"updateBy":"admin","configId":101}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:15:51');
INSERT INTO "sys_oper_log"
VALUES (275, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标授权","updateTime":1595300226000,"configType":"N","configValue":"4956de596a564ee3be14604d0a0e4e9e","params":{},"configKey":"dsjy_token","createBy":"admin","createTime":1595300120000,"updateBy":"admin","configId":105}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:15:55');
INSERT INTO "sys_oper_log"
VALUES (276, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标单价","remark":"金币","updateTime":1595311732000,"configType":"N","configValue":"3","params":{},"configKey":"single_bid_marking_price","createBy":"admin","createTime":1595311665000,"updateBy":"admin","configId":107}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:15:59');
INSERT INTO "sys_oper_log"
VALUES (277, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"打标成本","remark":"元","updateTime":1595311736000,"configType":"N","configValue":"0.06","params":{},"configKey":"single_cost_marking_price","createBy":"admin","createTime":1595311725000,"updateBy":"admin","configId":108}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:16:02');
INSERT INTO "sys_oper_log"
VALUES (278, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘流量单价","remark":"金币","configType":"Y","configValue":"3","params":{},"configKey":"single_bid_pdtflow_price","createBy":"admin","createTime":1595311864000,"updateBy":"admin","configId":109}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:16:07');
INSERT INTO "sys_oper_log"
VALUES (279, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘流量单价","remark":"金币","updateTime":1595312167000,"configType":"N","configValue":"3","params":{},"configKey":"single_bid_pdtflow_price","createBy":"admin","createTime":1595311864000,"updateBy":"admin","configId":109}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:16:11');
INSERT INTO "sys_oper_log"
VALUES (280, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘流量成本","remark":"元","configType":"N","configValue":"0.04","params":{},"configKey":"single_cost_pdtflow_price","createBy":"admin","createTime":1595311925000,"updateBy":"admin","configId":110}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:16:14');
INSERT INTO "sys_oper_log"
VALUES (281, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘APPID","configType":"N","configValue":"11217778","params":{},"configKey":"pdt_appid","createBy":"admin","createTime":1595312087000,"updateBy":"admin","configId":111}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:16:19');
INSERT INTO "sys_oper_log"
VALUES (282, '参数管理', 2, 'com.ruoyi.project.system.controller.SysConfigController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/config', '192.168.0.167', '内网IP',
        '{"configName":"派大淘密钥","configType":"N","configValue":"02479ea0b17e9cfbe5513dd8f9503dd4","params":{},"configKey":"pdt_appsecrect","createBy":"admin","createTime":1595312123000,"updateBy":"admin","configId":112}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 14:16:22');
INSERT INTO "sys_oper_log"
VALUES (283, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '192.168.0.167', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/ff9fb35f5d77647b307d83374c7df1af.jpeg","code":200}', 0,
        NULL, '2020-07-21 14:55:33');
INSERT INTO "sys_oper_log"
VALUES (284, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1,
        'java.io.FileNotFoundException: C:\Users\lurong\AppData\Local\Temp\tomcat.407827446951999160.8080\work\Tomcat\localhost\ROOT\upload\avatar\2020\07\21\13bafc7c022b296767522108bcaef653.jpeg (系统找不到指定的路径。)',
        '2020-07-21 17:23:44');
INSERT INTO "sys_oper_log"
VALUES (285, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1,
        'java.io.FileNotFoundException: C:\Users\lurong\AppData\Local\Temp\tomcat.1912346716855507494.8080\work\Tomcat\localhost\ROOT\upload\avatar\2020\07\21\37d61c6aaa36f1e790afd649af47bddb.jpeg (系统找不到指定的路径。)',
        '2020-07-21 17:27:03');
INSERT INTO "sys_oper_log"
VALUES (286, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1,
        'java.io.FileNotFoundException: C:\Users\lurong\AppData\Local\Temp\tomcat.1912346716855507494.8080\work\Tomcat\localhost\ROOT\upload\avatar\2020\07\21\9c314b1d1a988870498eb375ff133d2c.jpeg (系统找不到指定的路径。)',
        '2020-07-21 17:28:02');
INSERT INTO "sys_oper_log"
VALUES (287, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1,
        'java.io.FileNotFoundException: C:\Users\lurong\AppData\Local\Temp\tomcat.2937057753571602891.8080\work\Tomcat\localhost\ROOT\upload\avatar\2020\07\21\8ad3dfd5111e5dab4fd28504c60385d4.jpeg (系统找不到指定的路径。)',
        '2020-07-21 17:30:56');
INSERT INTO "sys_oper_log"
VALUES (288, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1,
        'java.io.FileNotFoundException: C:\Users\lurong\AppData\Local\Temp\tomcat.2937057753571602891.8080\work\Tomcat\localhost\ROOT\upload\avatar\2020\07\21\5c4dfdfcedd663561e7bf5fcaa1f4ea9.jpeg (系统找不到指定的路径。)',
        '2020-07-21 17:32:03');
INSERT INTO "sys_oper_log"
VALUES (289, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1,
        'java.io.FileNotFoundException: C:\Users\lurong\AppData\Local\Temp\tomcat.2937057753571602891.8080\work\Tomcat\localhost\ROOT\upload\avatar\2020\07\21\117c0ecdb778a370600ef4bfe175199e.jpeg (系统找不到指定的路径。)',
        '2020-07-21 17:33:24');
INSERT INTO "sys_oper_log"
VALUES (290, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1,
        'java.io.FileNotFoundException: C:\Users\lurong\AppData\Local\Temp\tomcat.1644414051595757687.8080\work\Tomcat\localhost\ROOT\upload\avatar\2020\07\21\6b475e892ed3c5afff3a3fbeb85569dd.jpeg (系统找不到指定的路径。)',
        '2020-07-21 17:39:41');
INSERT INTO "sys_oper_log"
VALUES (291, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/0922cb16c4dda5bf465bd57f4bd5373d.jpeg","code":200}', 0,
        NULL, '2020-07-21 17:47:08');
INSERT INTO "sys_oper_log"
VALUES (292, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/3122ea0c1202a56b0520b1dda80e57cb.jpeg","code":200}', 0,
        NULL, '2020-07-21 17:47:17');
INSERT INTO "sys_oper_log"
VALUES (293, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/e2451a533a2103898c3669d4cbb9ab0a.jpeg","code":200}', 0,
        NULL, '2020-07-21 17:48:54');
INSERT INTO "sys_oper_log"
VALUES (294, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/4aa98914b6ce541ae28afc66f6fbc37b.jpeg","code":200}', 0,
        NULL, '2020-07-21 17:51:13');
INSERT INTO "sys_oper_log"
VALUES (295, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/dc9f3ec24f078df84d6dcd1ba8551372.jpeg","code":200}', 0,
        NULL, '2020-07-21 17:51:22');
INSERT INTO "sys_oper_log"
VALUES (296, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/2471309c75b7cab89a42f6dacbb9e52e.jpeg","code":200}', 0,
        NULL, '2020-07-21 17:52:51');
INSERT INTO "sys_oper_log"
VALUES (297, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, NULL, NULL,
        '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1, '', '2020-07-21 18:53:55');
INSERT INTO "sys_oper_log"
VALUES (298, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/14c5493fb4b51719b440764409c446d3.jpeg","code":200}', 0,
        NULL, '2020-07-21 18:55:31');
INSERT INTO "sys_oper_log"
VALUES (299, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '192.168.0.167', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/ced2642f014dcb8672163892b6369097.jpeg","code":200}', 0,
        NULL, '2020-07-21 18:56:32');
INSERT INTO "sys_oper_log"
VALUES (300, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '', 'null', 1, '文件名、目录名或卷标语法不正确。',
        '2020-07-21 19:06:07');
INSERT INTO "sys_oper_log"
VALUES (301, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/c6a8b625f23b4e13eb7fd0763143d413.jpeg","code":200}', 0,
        NULL, '2020-07-21 19:06:53');
INSERT INTO "sys_oper_log"
VALUES (302, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '127.0.0.1', '内网IP',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15888888888","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"管理员","sex":"1","deptId":103,"avatar":"/profile/avatar/2020/07/21/14c5493fb4b51719b440764409c446d3.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-21 19:07:00');
INSERT INTO "sys_oper_log"
VALUES (303, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '127.0.0.1', '内网IP', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/21/429fd936f8cbe6b8968db211d298957f.jpeg","code":200}', 0,
        NULL, '2020-07-21 19:08:41');
INSERT INTO "sys_oper_log"
VALUES (304, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '58.62.31.240', 'XX XX', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/24/fee1590486855ce23fb08eab45388df1.jpeg","code":200}', 0,
        NULL, '2020-07-24 13:48:07');
INSERT INTO "sys_oper_log"
VALUES (305, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '58.62.31.240', 'XX XX',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15888888888","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"管理员","sex":"1","deptId":103,"avatar":"/profile/avatar/2020/07/21/429fd936f8cbe6b8968db211d298957f.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-24 13:48:09');
INSERT INTO "sys_oper_log"
VALUES (306, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '58.62.31.240', 'XX XX', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/24/e54d5c026540ec7b9ce1fadeda421efb.jpeg","code":200}', 0,
        NULL, '2020-07-24 13:52:54');
INSERT INTO "sys_oper_log"
VALUES (307, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '58.62.31.240', 'XX XX',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15888888888","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"管理员","sex":"1","deptId":103,"avatar":"/profile/avatar/2020/07/21/429fd936f8cbe6b8968db211d298957f.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-24 13:52:59');
INSERT INTO "sys_oper_log"
VALUES (308, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '58.62.31.240', 'XX XX',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15888888888","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"管理员","sex":"0","deptId":103,"avatar":"/profile/avatar/2020/07/21/429fd936f8cbe6b8968db211d298957f.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-24 13:54:05');
INSERT INTO "sys_oper_log"
VALUES (309, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '58.62.31.240', 'XX XX', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/24/6e53f25e531f61adc95f360e3b320e41.jpeg","code":200}', 0,
        NULL, '2020-07-24 13:54:45');
INSERT INTO "sys_oper_log"
VALUES (310, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '58.62.31.240', 'XX XX',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15111111111","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"管理员","sex":"0","deptId":103,"avatar":"/profile/avatar/2020/07/24/e54d5c026540ec7b9ce1fadeda421efb.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-24 13:55:05');
INSERT INTO "sys_oper_log"
VALUES (311, '用户头像', 2, 'com.ruoyi.project.system.controller.SysProfileController.avatar()', 'POST', 1, 'admin',
        NULL, '/system/user/profile/avatar', '113.115.54.231', 'XX XX', '',
        '{"msg":"操作成功","imgUrl":"/profile/avatar/2020/07/24/5418b5be3194e6a8734aa2f60d52f157.jpeg","code":200}', 0,
        NULL, '2020-07-24 14:10:29');
INSERT INTO "sys_oper_log"
VALUES (312, '个人信息', 2, 'com.ruoyi.project.system.controller.SysProfileController.updateProfile()', 'PUT', 1,
        'admin', NULL, '/system/user/profile', '113.115.54.231', 'XX XX',
        '{"roles":[{"flag":false,"roleId":1,"admin":true,"dataScope":"1","params":{},"roleSort":"1","roleKey":"admin","roleName":"管理员","status":"0"}],"phonenumber":"15111111111","admin":true,"loginDate":1521171180000,"remark":"管理员","delFlag":"0","password":"$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui","loginIp":"127.0.0.1","email":"ry@163.com","nickName":"管理员","sex":"0","deptId":103,"avatar":"/profile/avatar/2020/07/24/6e53f25e531f61adc95f360e3b320e41.jpeg","dept":{"deptName":"研发部门","leader":"若依","deptId":103,"orderNum":"1","params":{},"parentId":101,"children":[],"status":"0"},"params":{},"userName":"admin","userId":1,"createBy":"admin","createTime":1521171180000,"status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-24 14:10:32');
INSERT INTO "sys_oper_log"
VALUES (313, '参数管理', 2, 'com.ruoyi.project.dq.controller.AppUserController.edit()', 'POST', 1, 'admin', NULL,
        '/system/appUser/edit', '14.24.151.46', 'XX XX',
        '{"app_user_id":72,"username":"18073721771","account_status":"0","update_by":"admin"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-26 10:42:13');
INSERT INTO "sys_oper_log"
VALUES (314, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"peoples","orderNum":"1","menuName":"会员管理","params":{},"parentId":4,"path":"appuser","component":"dq/appuser/index","children":[],"createTime":1592618632000,"updateBy":"admin","isFrame":"1","menuId":2000,"menuType":"C","perms":"system:appUser:appUser","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:43:13');
INSERT INTO "sys_oper_log"
VALUES (315, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"money","orderNum":"2","menuName":"充值管理","params":{},"parentId":4,"path":"recharge","component":"dq/recharge/index","children":[],"createTime":1592638426000,"updateBy":"admin","isFrame":"1","menuId":2001,"menuType":"C","perms":"system:recharge:recharge","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:43:37');
INSERT INTO "sys_oper_log"
VALUES (316, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"server","orderNum":"3","menuName":"接口管理","params":{},"parentId":4,"path":"callrecord","component":"dq/callrecord/index","children":[],"createTime":1593521670000,"updateBy":"admin","isFrame":"1","menuId":2009,"menuType":"C","perms":"system:callrecord:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:44:04');
INSERT INTO "sys_oper_log"
VALUES (317, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"list","orderNum":"1","menuName":"打标记录","params":{},"parentId":2002,"path":"markingRecord","component":"dq/marking/index","children":[],"createTime":1594177707000,"updateBy":"admin","isFrame":"1","menuId":2012,"menuType":"C","perms":"system:markingRecord:markingRecord","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:45:05');
INSERT INTO "sys_oper_log"
VALUES (318, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"language","orderNum":"2","menuName":"验号记录","params":{},"parentId":2002,"path":"yhrecord","component":"dq/yhrecord/index","children":[],"createTime":1594179359000,"updateBy":"admin","isFrame":"1","menuId":2013,"menuType":"C","perms":"system:yhrecord:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:45:16');
INSERT INTO "sys_oper_log"
VALUES (319, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"international","orderNum":"3","menuName":"电商教父流量记录","params":{},"parentId":2002,"path":"flowrecord","component":"dq/flowrecord/index","children":[],"createTime":1594179406000,"updateBy":"admin","isFrame":"1","menuId":2014,"menuType":"C","perms":"system:flowrecord:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:45:32');
INSERT INTO "sys_oper_log"
VALUES (320, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"job","orderNum":"4","menuName":"派大淘流量记录","params":{},"parentId":2002,"path":"pdtflow","component":"dq/pdtflow/index","children":[],"createTime":1594179464000,"updateBy":"admin","isFrame":"1","menuId":2015,"menuType":"C","perms":"system:pdtflow:list","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:45:41');
INSERT INTO "sys_oper_log"
VALUES (321, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"monitor","orderNum":"1","menuName":"验号统计","params":{},"parentId":2005,"path":"yhstatistics","component":"dq/statistics/yhindex","children":[],"createTime":1592723780000,"updateBy":"admin","isFrame":"1","menuId":2006,"menuType":"C","perms":"system:dsjfstatistics:yhlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:45:54');
INSERT INTO "sys_oper_log"
VALUES (322, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"component","orderNum":"2","menuName":"流量统计","params":{},"parentId":2005,"path":"flowstatistics","component":"dq/statistics/flowindex","children":[],"createTime":1592723829000,"updateBy":"admin","isFrame":"1","menuId":2007,"menuType":"C","perms":"system:dsjfstatistics:flowlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:46:04');
INSERT INTO "sys_oper_log"
VALUES (323, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"message","orderNum":"3","menuName":"派大淘流量统计","params":{},"parentId":2005,"path":"pdtstatistics","component":"dq/statistics/pdtflowindex","children":[],"createTime":1593509136000,"updateBy":"admin","isFrame":"1","menuId":2008,"menuType":"C","perms":"system:pdtstatistics:flowlist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:46:11');
INSERT INTO "sys_oper_log"
VALUES (324, '菜单管理', 2, 'com.ruoyi.project.system.controller.SysMenuController.edit()', 'PUT', 1, 'admin', NULL,
        '/system/menu', '58.62.29.238', 'XX XX',
        '{"visible":"0","icon":"nested","orderNum":"4","menuName":"打标统计","params":{},"parentId":2005,"path":"makinglist","component":"dq/statistics/dbindex","children":[],"createTime":1594176482000,"updateBy":"admin","isFrame":"1","menuId":2011,"menuType":"C","perms":"system:dsjfstatistics:makinglist","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:46:20');
INSERT INTO "sys_oper_log"
VALUES (325, '定时任务', 2, 'com.ruoyi.project.monitor.controller.SysJobController.edit()', 'PUT', 1, 'admin', NULL,
        '/monitor/job', '58.62.29.238', 'XX XX',
        '{"jobName":"派大淘流量任务","concurrent":"0","remark":"","jobGroup":"SYSTEM","params":{},"cronExpression":"0 */10 * * * ?","jobId":100,"createBy":"","nextValidTime":1595908200000,"createTime":1593678646000,"invokeTarget":"dqTask.checkPdtFlowData","misfirePolicy":"1","status":"0"}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 11:48:44');
INSERT INTO "sys_oper_log"
VALUES (326, '用户管理', 2, 'com.ruoyi.project.system.controller.SysUserController.resetPwd()', 'PUT', 1, 'admin', NULL,
        '/system/user/resetPwd', '58.62.29.238', 'XX XX',
        '{"admin":false,"password":"$2a$10$B/8GZqpKNjQ6KvJwInvhdOXI1gdK2eGVam8a9A9pPh1bLUPC7PYLm","updateBy":"admin","params":{},"userId":2}',
        '{"msg":"操作成功","code":200}', 0, NULL, '2020-07-28 14:06:07');

-- ----------------------------
-- Table structure for sys_post
-- ----------------------------
DROP TABLE IF EXISTS "sys_post";
CREATE TABLE "sys_post"
(
    "post_id"     integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "post_code"   text(64) NOT NULL,
    "post_name"   text(50) NOT NULL,
    "post_sort"   integer(11) NOT NULL,
    "status"      text(1) NOT NULL,
    "create_by"   text(64),
    "create_time" text,
    "update_by"   text(64),
    "update_time" text,
    "remark"      text(500)
);

-- ----------------------------
-- Records of sys_post
-- ----------------------------
INSERT INTO "sys_post"
VALUES (1, 'ceo', '董事长', 1, 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_post"
VALUES (2, 'se', '项目经理', 2, 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_post"
VALUES (3, 'hr', '人力资源', 3, 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');
INSERT INTO "sys_post"
VALUES (4, 'user', '普通员工', 4, 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '');

-- ----------------------------
-- Table structure for sys_role
-- ----------------------------
DROP TABLE IF EXISTS "sys_role";
CREATE TABLE "sys_role"
(
    "role_id"     integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "role_name"   text(30) NOT NULL,
    "role_key"    text(100) NOT NULL,
    "role_sort"   integer(11) NOT NULL,
    "data_scope"  text(1),
    "status"      text(1) NOT NULL,
    "del_flag"    text(1),
    "create_by"   text(64),
    "create_time" text,
    "update_by"   text(64),
    "update_time" text,
    "remark"      text(500)
);

-- ----------------------------
-- Records of sys_role
-- ----------------------------
INSERT INTO "sys_role"
VALUES (1, '管理员', 'admin', 1, 1, 0, 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '管理员');
INSERT INTO "sys_role"
VALUES (2, '普通角色', 'common', 2, 2, 0, 0, 'admin', '2018-03-16 11:33:00', 'ry', '2018-03-16 11:33:00', '普通角色');

-- ----------------------------
-- Table structure for sys_role_dept
-- ----------------------------
DROP TABLE IF EXISTS "sys_role_dept";
CREATE TABLE "sys_role_dept"
(
    "role_id" integer(20) NOT NULL,
    "dept_id" integer(20) NOT NULL,
    PRIMARY KEY ("role_id", "dept_id")
);

-- ----------------------------
-- Records of sys_role_dept
-- ----------------------------
INSERT INTO "sys_role_dept"
VALUES (2, 100);
INSERT INTO "sys_role_dept"
VALUES (2, 101);
INSERT INTO "sys_role_dept"
VALUES (2, 105);

-- ----------------------------
-- Table structure for sys_role_menu
-- ----------------------------
DROP TABLE IF EXISTS "sys_role_menu";
CREATE TABLE "sys_role_menu"
(
    "role_id" integer(20) NOT NULL,
    "menu_id" integer(20) NOT NULL,
    PRIMARY KEY ("role_id", "menu_id")
);

-- ----------------------------
-- Records of sys_role_menu
-- ----------------------------
INSERT INTO "sys_role_menu"
VALUES (2, 1);
INSERT INTO "sys_role_menu"
VALUES (2, 2);
INSERT INTO "sys_role_menu"
VALUES (2, 3);
INSERT INTO "sys_role_menu"
VALUES (2, 4);
INSERT INTO "sys_role_menu"
VALUES (2, 100);
INSERT INTO "sys_role_menu"
VALUES (2, 101);
INSERT INTO "sys_role_menu"
VALUES (2, 102);
INSERT INTO "sys_role_menu"
VALUES (2, 103);
INSERT INTO "sys_role_menu"
VALUES (2, 104);
INSERT INTO "sys_role_menu"
VALUES (2, 105);
INSERT INTO "sys_role_menu"
VALUES (2, 106);
INSERT INTO "sys_role_menu"
VALUES (2, 107);
INSERT INTO "sys_role_menu"
VALUES (2, 108);
INSERT INTO "sys_role_menu"
VALUES (2, 109);
INSERT INTO "sys_role_menu"
VALUES (2, 110);
INSERT INTO "sys_role_menu"
VALUES (2, 111);
INSERT INTO "sys_role_menu"
VALUES (2, 112);
INSERT INTO "sys_role_menu"
VALUES (2, 113);
INSERT INTO "sys_role_menu"
VALUES (2, 114);
INSERT INTO "sys_role_menu"
VALUES (2, 115);
INSERT INTO "sys_role_menu"
VALUES (2, 500);
INSERT INTO "sys_role_menu"
VALUES (2, 501);
INSERT INTO "sys_role_menu"
VALUES (2, 1000);
INSERT INTO "sys_role_menu"
VALUES (2, 1001);
INSERT INTO "sys_role_menu"
VALUES (2, 1002);
INSERT INTO "sys_role_menu"
VALUES (2, 1003);
INSERT INTO "sys_role_menu"
VALUES (2, 1004);
INSERT INTO "sys_role_menu"
VALUES (2, 1005);
INSERT INTO "sys_role_menu"
VALUES (2, 1006);
INSERT INTO "sys_role_menu"
VALUES (2, 1007);
INSERT INTO "sys_role_menu"
VALUES (2, 1008);
INSERT INTO "sys_role_menu"
VALUES (2, 1009);
INSERT INTO "sys_role_menu"
VALUES (2, 1010);
INSERT INTO "sys_role_menu"
VALUES (2, 1011);
INSERT INTO "sys_role_menu"
VALUES (2, 1012);
INSERT INTO "sys_role_menu"
VALUES (2, 1013);
INSERT INTO "sys_role_menu"
VALUES (2, 1014);
INSERT INTO "sys_role_menu"
VALUES (2, 1015);
INSERT INTO "sys_role_menu"
VALUES (2, 1016);
INSERT INTO "sys_role_menu"
VALUES (2, 1017);
INSERT INTO "sys_role_menu"
VALUES (2, 1018);
INSERT INTO "sys_role_menu"
VALUES (2, 1019);
INSERT INTO "sys_role_menu"
VALUES (2, 1020);
INSERT INTO "sys_role_menu"
VALUES (2, 1021);
INSERT INTO "sys_role_menu"
VALUES (2, 1022);
INSERT INTO "sys_role_menu"
VALUES (2, 1023);
INSERT INTO "sys_role_menu"
VALUES (2, 1024);
INSERT INTO "sys_role_menu"
VALUES (2, 1025);
INSERT INTO "sys_role_menu"
VALUES (2, 1026);
INSERT INTO "sys_role_menu"
VALUES (2, 1027);
INSERT INTO "sys_role_menu"
VALUES (2, 1028);
INSERT INTO "sys_role_menu"
VALUES (2, 1029);
INSERT INTO "sys_role_menu"
VALUES (2, 1030);
INSERT INTO "sys_role_menu"
VALUES (2, 1031);
INSERT INTO "sys_role_menu"
VALUES (2, 1032);
INSERT INTO "sys_role_menu"
VALUES (2, 1033);
INSERT INTO "sys_role_menu"
VALUES (2, 1034);
INSERT INTO "sys_role_menu"
VALUES (2, 1035);
INSERT INTO "sys_role_menu"
VALUES (2, 1036);
INSERT INTO "sys_role_menu"
VALUES (2, 1037);
INSERT INTO "sys_role_menu"
VALUES (2, 1038);
INSERT INTO "sys_role_menu"
VALUES (2, 1039);
INSERT INTO "sys_role_menu"
VALUES (2, 1040);
INSERT INTO "sys_role_menu"
VALUES (2, 1041);
INSERT INTO "sys_role_menu"
VALUES (2, 1042);
INSERT INTO "sys_role_menu"
VALUES (2, 1043);
INSERT INTO "sys_role_menu"
VALUES (2, 1044);
INSERT INTO "sys_role_menu"
VALUES (2, 1045);
INSERT INTO "sys_role_menu"
VALUES (2, 1046);
INSERT INTO "sys_role_menu"
VALUES (2, 1047);
INSERT INTO "sys_role_menu"
VALUES (2, 1048);
INSERT INTO "sys_role_menu"
VALUES (2, 1049);
INSERT INTO "sys_role_menu"
VALUES (2, 1050);
INSERT INTO "sys_role_menu"
VALUES (2, 1051);
INSERT INTO "sys_role_menu"
VALUES (2, 1052);
INSERT INTO "sys_role_menu"
VALUES (2, 1053);
INSERT INTO "sys_role_menu"
VALUES (2, 1054);
INSERT INTO "sys_role_menu"
VALUES (2, 1055);
INSERT INTO "sys_role_menu"
VALUES (2, 1056);
INSERT INTO "sys_role_menu"
VALUES (2, 1057);
INSERT INTO "sys_role_menu"
VALUES (2, 1058);
INSERT INTO "sys_role_menu"
VALUES (2, 1059);
INSERT INTO "sys_role_menu"
VALUES (2, 1060);

-- ----------------------------
-- Table structure for sys_user
-- ----------------------------
DROP TABLE IF EXISTS "sys_user";
CREATE TABLE "sys_user"
(
    "user_id"     integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    "dept_id"     integer(20),
    "user_name"   text(30) NOT NULL,
    "nick_name"   text(30) NOT NULL,
    "user_type"   text(2),
    "email"       text(50),
    "phonenumber" text(11),
    "sex"         text(1),
    "avatar"      text(100),
    "password"    text(100),
    "status"      text(1),
    "del_flag"    text(1),
    "login_ip"    text(50),
    "login_date"  text,
    "create_by"   text(64),
    "create_time" text,
    "update_by"   text(64),
    "update_time" text,
    "remark"      text(500)
);

-- ----------------------------
-- Records of sys_user
-- ----------------------------
INSERT INTO "sys_user"
VALUES (1, 103, 'admin', '管理员', '00', 'ry@163.com', 15111111111, 0,
        '/profile/avatar/2020/07/24/6e53f25e531f61adc95f360e3b320e41.jpeg',
        '$2a$10$1HfvY0HlWMJqr/Gfvmnfm.w1A8n7PYkt9dWFiO5aJnLqhOmnQRnui', 0, 0, '127.0.0.1', '2018-03-16 11:33:00',
        'admin', '2018-03-16 11:33:00', 'ry', '2020-07-24 14:10:32', '管理员');
INSERT INTO "sys_user"
VALUES (2, 105, 'ry', '若依', '00', 'ry@qq.com', 15666666666, 1, '',
        '$2a$10$B/8GZqpKNjQ6KvJwInvhdOXI1gdK2eGVam8a9A9pPh1bLUPC7PYLm', 1, 0, '127.0.0.1', '2018-03-16 11:33:00',
        'admin', '2018-03-16 11:33:00', 'admin', '2020-07-28 14:06:07', '测试员');

-- ----------------------------
-- Table structure for sys_user_post
-- ----------------------------
DROP TABLE IF EXISTS "sys_user_post";
CREATE TABLE "sys_user_post"
(
    "user_id" integer(20) NOT NULL,
    "post_id" integer(20) NOT NULL,
    PRIMARY KEY ("user_id", "post_id")
);

-- ----------------------------
-- Records of sys_user_post
-- ----------------------------
INSERT INTO "sys_user_post"
VALUES (1, 1);
INSERT INTO "sys_user_post"
VALUES (2, 2);

-- ----------------------------
-- Table structure for sys_user_role
-- ----------------------------
DROP TABLE IF EXISTS "sys_user_role";
CREATE TABLE "sys_user_role"
(
    "user_id" integer(20) NOT NULL,
    "role_id" integer(20) NOT NULL,
    PRIMARY KEY ("user_id", "role_id")
);

-- ----------------------------
-- Records of sys_user_role
-- ----------------------------
INSERT INTO "sys_user_role"
VALUES (1, 1);
INSERT INTO "sys_user_role"
VALUES (2, 2);

-- ----------------------------
-- Auto increment value for dq_app_user
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 72
WHERE name = 'dq_app_user';

-- ----------------------------
-- Auto increment value for gen_table
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 5
WHERE name = 'gen_table';

-- ----------------------------
-- Auto increment value for gen_table_column
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 81
WHERE name = 'gen_table_column';

-- ----------------------------
-- Indexes structure for table qrtz_triggers
-- ----------------------------
CREATE INDEX "sched_name"
    ON "qrtz_triggers" (
                        "sched_name" ASC,
                        "job_name" ASC,
                        "job_group" ASC
        );

-- ----------------------------
-- Auto increment value for sys_config
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 112
WHERE name = 'sys_config';

-- ----------------------------
-- Auto increment value for sys_dept
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 109
WHERE name = 'sys_dept';

-- ----------------------------
-- Auto increment value for sys_dict_data
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 137
WHERE name = 'sys_dict_data';

-- ----------------------------
-- Auto increment value for sys_dict_type
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 109
WHERE name = 'sys_dict_type';

-- ----------------------------
-- Indexes structure for table sys_dict_type
-- ----------------------------
CREATE UNIQUE INDEX "dict_type"
    ON "sys_dict_type" (
                        "dict_type" ASC
        );

-- ----------------------------
-- Auto increment value for sys_job_log
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 83834
WHERE name = 'sys_job_log';

-- ----------------------------
-- Auto increment value for sys_logininfor
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 544
WHERE name = 'sys_logininfor';

-- ----------------------------
-- Auto increment value for sys_menu
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 2015
WHERE name = 'sys_menu';

-- ----------------------------
-- Auto increment value for sys_notice
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 3
WHERE name = 'sys_notice';

-- ----------------------------
-- Auto increment value for sys_oper_log
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 326
WHERE name = 'sys_oper_log';

-- ----------------------------
-- Auto increment value for sys_post
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 4
WHERE name = 'sys_post';

-- ----------------------------
-- Auto increment value for sys_role
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 2
WHERE name = 'sys_role';

-- ----------------------------
-- Auto increment value for sys_user
-- ----------------------------
UPDATE "sqlite_sequence"
SET seq = 2
WHERE name = 'sys_user';

PRAGMA
foreign_keys = true;