-- MySQL dump 10.13  Distrib 8.0.11, for macos10.13 (x86_64)
--
-- Host: localhost    Database: eos
-- ------------------------------------------------------
-- Server version	8.0.11

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
 SET NAMES utf8mb4 ;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Create database `eos`
--

DROP DATABASE IF EXISTS `eos`;
CREATE DATABASE `eos` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE eos;

--
-- Table structure for table `accounts`
--

DROP TABLE IF EXISTS `accounts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `accounts` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `name` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '账户名',
  `abi` json DEFAULT NULL COMMENT 'abi 信息',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_accounts_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `accounts_keys`
--

DROP TABLE IF EXISTS `accounts_keys`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `accounts_keys` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `account` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '账户名',
  `public_key` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '公钥',
  `permission` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '权限名称',
  PRIMARY KEY (`id`),
  KEY `account` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `actions`
--

DROP TABLE IF EXISTS `actions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `actions` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `account` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '合约拥有者账号',
  `transaction_id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '交易号',
  `seq` smallint(6) NOT NULL DEFAULT '0' COMMENT '序列号',
  `parent` bigint(20) NOT NULL DEFAULT '0' COMMENT '',
  `name` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'action 名称',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `data` json DEFAULT NULL COMMENT 'action 数据',
  `authorization` mediumtext DEFAULT NULL COMMENT '执行权限',
  `eosto` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '提取 data 的 to 字段',
  `eosfrom` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '提取 data 的 from 字段',
  `receiver` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '提取 data 的 receiver 字段',
  `payer` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '提取 data 的 payer 字段',
  `newaccount` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '新建账号名称',
  `sellram_account` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '卖内存的用户名',
  PRIMARY KEY (`id`),
  KEY `idx_actions_account` (`account`),
  KEY `idx_actions_name` (`name`),
  KEY `idx_actions_tx_id` (`transaction_id`),
  KEY `idx_actions_created` (`created_at`),
  KEY `idx_actions_eosto` (`eosto`),
  KEY `idx_actions_eosfrom` (`eosfrom`),
  KEY `idx_actions_receiver` (`receiver`),
  KEY `idx_actions_payer` (`payer`),
  KEY `idx_actions_newaccount` (`newaccount`),
  KEY `idx_actions_sellram_account` (`sellram_account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `actions_accounts`
--

-- DROP TABLE IF EXISTS `actions_accounts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
-- SET character_set_client = utf8mb4 ;
-- CREATE TABLE `actions_accounts` (
--   `id` bigint(20) NOT NULL AUTO_INCREMENT,
--   `actor` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
--   `permission` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
--   `action_id` bigint(20) NOT NULL DEFAULT '0',
--   PRIMARY KEY (`id`),
--   KEY `idx_actions_actor` (`actor`),
--   KEY `idx_actions_action_id` (`action_id`)
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `assets`
--

DROP TABLE IF EXISTS `assets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `assets` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `supply` bigint(65) NOT NULL DEFAULT '0' COMMENT '已发行的 Token 量',
  `max_supply` bigint(65) NOT NULL DEFAULT '0' COMMENT '总 Token 量',
  `symbol_precision` int(2) NOT NULL DEFAULT '0' COMMENT 'Token 资产精度',
  `symbol` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Token 资产符号',
  `issuer` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Token 拥有者',
  `contract_owner` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Token 合约拥有者',
  `logo_url` varchar(200) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Token Logo 链接',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_symbol_owner` (`symbol`,`contract_owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `blocks`
--

DROP TABLE IF EXISTS `blocks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `blocks` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `block_id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '块号',
  `block_number` bigint(20) NOT NULL DEFAULT '0' COMMENT '块序列号',
  `prev_block_id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '上一个块的块号',
  `irreversible` tinyint(1) NOT NULL DEFAULT '0' COMMENT '是否不可逆',
  `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `transaction_merkle_root` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '区块交易的 merkel 根',
  `action_merkle_root` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '区块 action 的 merkel 根',
  `producer` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '区块生产者',
  `version` int(11) NOT NULL DEFAULT '0'  COMMENT '版本号',
  `new_producers` json DEFAULT NULL  COMMENT '新的区块生产者',
  `num_transactions` int(11) NOT NULL DEFAULT '0'  COMMENT '区块生产的交易数量',
  `confirmed` int(11) NOT NULL DEFAULT '0'  COMMENT '区块确认数',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_block_id` (`block_id`),
  KEY `idx_blocks_producer` (`producer`),
  KEY `idx_prev_block_id` (`prev_block_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `refunds`
--

DROP TABLE IF EXISTS `refunds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `refunds` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `owner` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '资源拥有者',
  `request_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `net_amount` bigint(65) NOT NULL DEFAULT '0' COMMENT '赎回的 net 资源',
  `cpu_amount` bigint(65) NOT NULL DEFAULT '0' COMMENT '赎回的 cpu 资源',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_refunds_owner` (`owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `stakes`
--

DROP TABLE IF EXISTS `stakes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `stakes` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `account` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '账号',
  `cpu_amount_by_self` bigint(65) NOT NULL DEFAULT '0' COMMENT '自己抵押的 cpu 资源',
  `net_amount_by_self` bigint(65) NOT NULL DEFAULT '0' COMMENT '自己抵押的 net 资源',
  `cpu_amount_by_other` bigint(65) NOT NULL DEFAULT '0' COMMENT '别人帮忙抵押的 cpu 资源',
  `net_amount_by_other` bigint(65) NOT NULL DEFAULT '0' COMMENT '别人帮忙抵押的 net 资源',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_stakes_account` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `tokens`
--

DROP TABLE IF EXISTS `tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `tokens` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `account` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '账号',
  `symbol` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Token 资产符号',
  `balance` bigint(65) NOT NULL DEFAULT '0' COMMENT 'Token 余额',
  `symbol_precision` int(2) NOT NULL DEFAULT '0' COMMENT 'Token 资产精度',
  `contract_owner` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Token 合约拥有者',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_symbol_owner_account` (`account`,`symbol`,`contract_owner`),
  KEY `idx_tokens_account` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `traces`
--

DROP TABLE IF EXISTS `traces`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `traces` (
  `tx_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '交易号',
  `data` mediumtext DEFAULT NULL COMMENT '交易结果集',
  `irreversible` tinyint(1) NOT NULL DEFAULT '0' COMMENT '是否不可逆',
  PRIMARY KEY (`tx_id`),
  UNIQUE KEY `idx_transactions_id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transactions`
--

DROP TABLE IF EXISTS `transactions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `transactions` (
  `tx_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '区块交易的 merkel 根',
  `block_num` bigint(20) NOT NULL DEFAULT '0' COMMENT '交易的区块号',
  `ref_block_num` bigint(20) NOT NULL DEFAULT '0' COMMENT '交易发起时的依赖区块号',
  `ref_block_prefix` bigint(20) NOT NULL DEFAULT '0' COMMENT '交易发起时的依赖区块前缀',
  `expiration` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '交易的过期时间',
  `pending` tinyint(1) NOT NULL DEFAULT '0' COMMENT '是否等待状态',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `num_actions` bigint(20) NOT NULL DEFAULT '0' COMMENT '交易中 action 的数量',
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '更新时间',
  `irreversible` tinyint(1) NOT NULL DEFAULT '0' COMMENT '是否不可逆',
  PRIMARY KEY (`tx_id`),
  UNIQUE KEY `idx_transactions_id` (`id`),
  KEY `transactions_block_num` (`block_num`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `votes`
--

DROP TABLE IF EXISTS `votes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `votes` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `voter` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '账户名',
  `proxy` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT '账号的投票代理人',
  `producers` json DEFAULT NULL COMMENT '账号所投节点的列表',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_votes_voter` (`voter`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `proposal`
--

DROP TABLE IF EXISTS `proposal`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `proposal` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `proposer` varchar(16) NOT NULL DEFAULT '0' COMMENT '多签发起者',
  `proposal_name` varchar(16) NOT NULL DEFAULT '0' COMMENT '多签提案名称',
  `requested_approvals` text NOT NULL COMMENT '多签提案需要的权限',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_proposer_proposal_name` (`proposer`,`proposal_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-07-23 15:41:58