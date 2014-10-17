-- --------------------------------------------------------
-- Host:                         192.168.1.8
-- Server version:               5.5.38-0ubuntu0.14.04.1 - (Ubuntu)
-- Server OS:                    debian-linux-gnu
-- HeidiSQL Version:             8.3.0.4834
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for table centini.groups
CREATE TABLE IF NOT EXISTS `groups` (
  `group` char(16) NOT NULL,
  `description` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`group`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.groups: ~3 rows (approximately)
/*!40000 ALTER TABLE `groups` DISABLE KEYS */;
INSERT INTO `groups` (`group`, `description`) VALUES
	('group1', 'Test Group 1'),
	('group2', 'Test Group 2'),
	('group3', 'Test Group 3');
/*!40000 ALTER TABLE `groups` ENABLE KEYS */;


-- Dumping structure for table centini.queues
CREATE TABLE IF NOT EXISTS `queues` (
  `id` tinyint(3) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.queues: ~0 rows (approximately)
/*!40000 ALTER TABLE `queues` DISABLE KEYS */;
/*!40000 ALTER TABLE `queues` ENABLE KEYS */;


-- Dumping structure for table centini.queue_log
CREATE TABLE IF NOT EXISTS `queue_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.queue_log: ~0 rows (approximately)
/*!40000 ALTER TABLE `queue_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `queue_log` ENABLE KEYS */;


-- Dumping structure for table centini.users
CREATE TABLE IF NOT EXISTS `users` (
  `username` char(16) NOT NULL,
  `password` varchar(32) NOT NULL,
  `fullname` varchar(50) DEFAULT NULL,
  `level` enum('Administrator','Agent','Supervisor','Manager') NOT NULL DEFAULT 'Agent',
  `peer` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.users: ~5 rows (approximately)
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` (`username`, `password`, `fullname`, `level`, `peer`) VALUES
	('admin', '0192023a7bbd73250516f069df18b500', 'Administrator', 'Administrator', NULL),
	('agent1', '6dfb015b52c29bbcaf9dc3f1d8f1a7be', 'Test Agent Satu', 'Agent', NULL),
	('agent2', '6dfb015b52c29bbcaf9dc3f1d8f1a7be', 'Test Agent Dua', 'Agent', NULL),
	('manager1', 'b2e39a830f1474354e987c6820fa98ac', 'Test Manager Satu', 'Manager', NULL),
	('spv1', 'c44303d24fc25a450c753a64371cf1d1', 'Test Supervisor Satu', 'Supervisor', NULL);
/*!40000 ALTER TABLE `users` ENABLE KEYS */;


-- Dumping structure for table centini.user_call_log
CREATE TABLE IF NOT EXISTS `user_call_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user_call_log: ~0 rows (approximately)
/*!40000 ALTER TABLE `user_call_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `user_call_log` ENABLE KEYS */;


-- Dumping structure for table centini.user_group
CREATE TABLE IF NOT EXISTS `user_group` (
  `id` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `username` char(16) NOT NULL,
  `group` char(16) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `username_group` (`username`,`group`),
  KEY `user_group_fk2` (`group`),
  CONSTRAINT `user_group_fk1` FOREIGN KEY (`username`) REFERENCES `users` (`username`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `user_group_fk2` FOREIGN KEY (`group`) REFERENCES `groups` (`group`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user_group: ~4 rows (approximately)
/*!40000 ALTER TABLE `user_group` DISABLE KEYS */;
INSERT INTO `user_group` (`id`, `username`, `group`) VALUES
	(1, 'agent1', 'group1'),
	(4, 'agent2', 'group3'),
	(2, 'spv1', 'group1'),
	(3, 'spv1', 'group2');
/*!40000 ALTER TABLE `user_group` ENABLE KEYS */;


-- Dumping structure for table centini.user_pause_log
CREATE TABLE IF NOT EXISTS `user_pause_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` char(16) DEFAULT NULL,
  `start` datetime NOT NULL,
  `finish` datetime DEFAULT NULL,
  `reason` varchar(50) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `user_pause_log_fk1` (`username`),
  KEY `reason` (`reason`),
  CONSTRAINT `user_pause_log_fk1` FOREIGN KEY (`username`) REFERENCES `users` (`username`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user_pause_log: ~4 rows (approximately)
/*!40000 ALTER TABLE `user_pause_log` DISABLE KEYS */;
INSERT INTO `user_pause_log` (`id`, `username`, `start`, `finish`, `reason`) VALUES
	(1, 'agent1', '2014-10-15 12:30:39', NULL, 'Test Doank'),
	(2, 'agent1', '2014-10-15 13:04:40', '2014-10-15 13:05:10', 'Test Doank'),
	(3, 'agent1', '2014-10-15 13:08:40', '2014-10-15 14:01:31', 'Test Doank'),
	(4, 'agent1', '2014-10-15 14:39:16', NULL, 'Test Doank');
/*!40000 ALTER TABLE `user_pause_log` ENABLE KEYS */;


-- Dumping structure for table centini.user_session_log
CREATE TABLE IF NOT EXISTS `user_session_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` char(16) DEFAULT NULL,
  `start` datetime NOT NULL,
  `finish` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `user_session_log_fk1` (`username`),
  CONSTRAINT `user_session_log_fk1` FOREIGN KEY (`username`) REFERENCES `users` (`username`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=82 DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user_session_log: ~81 rows (approximately)
/*!40000 ALTER TABLE `user_session_log` DISABLE KEYS */;
INSERT INTO `user_session_log` (`id`, `username`, `start`, `finish`) VALUES
	(1, 'agent1', '2014-10-14 12:52:21', NULL),
	(2, 'agent1', '2014-10-14 12:56:23', NULL),
	(3, 'agent1', '2014-10-14 12:58:16', NULL),
	(4, 'agent1', '2014-10-14 12:59:22', NULL),
	(5, 'agent1', '2014-10-14 13:00:52', NULL),
	(6, 'agent1', '2014-10-14 13:02:21', NULL),
	(7, 'agent1', '2014-10-14 13:05:18', NULL),
	(8, 'agent1', '2014-10-14 13:05:54', NULL),
	(9, 'agent1', '2014-10-14 13:06:35', NULL),
	(10, 'agent1', '2014-10-14 13:09:34', NULL),
	(11, 'agent1', '2014-10-14 13:13:52', NULL),
	(12, 'agent1', '2014-10-14 13:15:48', NULL),
	(13, 'spv1', '2014-10-14 13:22:54', '2014-10-14 13:24:23'),
	(14, 'agent1', '2014-10-14 13:23:09', '2014-10-14 13:23:44'),
	(15, 'agent1', '2014-10-14 14:22:31', '2014-10-14 14:22:45'),
	(16, 'agent1', '2014-10-14 15:02:55', NULL),
	(17, 'spv1', '2014-10-14 15:03:38', NULL),
	(18, 'agent1', '2014-10-14 15:06:40', '2014-10-14 15:10:58'),
	(19, 'spv1', '2014-10-14 15:07:35', '2014-10-14 15:11:11'),
	(20, 'spv1', '2014-10-14 15:11:42', '2014-10-14 15:16:59'),
	(21, 'agent1', '2014-10-14 15:12:11', '2014-10-14 15:16:43'),
	(22, 'agent1', '2014-10-14 15:19:07', NULL),
	(23, 'agent1', '2014-10-14 15:26:45', NULL),
	(24, 'agent1', '2014-10-14 15:32:11', NULL),
	(25, 'agent1', '2014-10-15 10:01:12', '2014-10-15 10:01:17'),
	(26, 'agent1', '2014-10-15 10:01:19', '2014-10-15 10:03:10'),
	(27, 'agent1', '2014-10-15 10:09:28', '2014-10-15 10:16:07'),
	(28, 'agent1', '2014-10-15 11:40:16', '2014-10-15 11:40:53'),
	(29, 'agent1', '2014-10-15 11:40:56', '2014-10-15 11:42:05'),
	(30, 'agent1', '2014-10-15 11:44:23', '2014-10-15 11:44:45'),
	(31, 'agent1', '2014-10-15 11:45:39', '2014-10-15 11:46:33'),
	(32, 'agent1', '2014-10-15 11:48:10', '2014-10-15 11:48:26'),
	(33, 'agent1', '2014-10-15 11:56:04', '2014-10-15 11:57:23'),
	(34, 'agent1', '2014-10-15 11:58:52', NULL),
	(35, 'agent1', '2014-10-15 12:00:21', NULL),
	(36, 'agent1', '2014-10-15 12:02:43', NULL),
	(37, 'agent1', '2014-10-15 12:02:43', '2014-10-15 12:02:51'),
	(38, 'agent1', '2014-10-15 12:02:54', NULL),
	(39, 'agent1', '2014-10-15 12:04:50', NULL),
	(40, 'agent1', '2014-10-15 12:09:16', NULL),
	(41, 'agent1', '2014-10-15 12:11:07', '2014-10-15 12:12:38'),
	(42, 'agent1', '2014-10-15 12:14:04', '2014-10-15 12:26:51'),
	(43, 'agent1', '2014-10-15 12:27:35', '2014-10-15 12:31:20'),
	(44, 'agent1', '2014-10-15 12:31:30', '2014-10-15 12:54:15'),
	(45, 'agent1', '2014-10-15 12:56:39', '2014-10-15 12:57:34'),
	(46, 'agent1', '2014-10-15 13:04:26', '2014-10-15 14:01:32'),
	(47, 'agent1', '2014-10-15 14:12:52', '2014-10-15 14:13:09'),
	(48, 'agent1', '2014-10-15 14:37:20', '2014-10-15 14:37:32'),
	(49, 'agent1', '2014-10-15 14:37:57', NULL),
	(50, 'agent1', '2014-10-16 10:00:24', '2014-10-16 11:37:20'),
	(51, 'agent1', '2014-10-16 12:07:42', '2014-10-16 12:07:45'),
	(52, 'spv1', '2014-10-16 12:08:30', NULL),
	(53, 'agent1', '2014-10-16 12:08:43', '2014-10-16 12:08:53'),
	(54, 'spv1', '2014-10-16 12:11:33', '2014-10-16 12:16:02'),
	(55, 'agent1', '2014-10-16 12:11:58', '2014-10-16 12:12:12'),
	(56, 'agent2', '2014-10-16 12:12:20', '2014-10-16 12:16:06'),
	(57, 'manager1', '2014-10-16 12:13:34', '2014-10-16 12:15:59'),
	(58, 'agent1', '2014-10-16 12:15:04', '2014-10-16 12:15:54'),
	(59, 'spv1', '2014-10-16 12:17:09', NULL),
	(60, 'agent1', '2014-10-16 12:19:28', '2014-10-16 12:19:35'),
	(61, 'manager1', '2014-10-16 12:19:58', NULL),
	(62, 'agent1', '2014-10-16 12:20:23', NULL),
	(63, 'agent1', '2014-10-16 12:25:44', '2014-10-16 12:26:41'),
	(64, 'agent1', '2014-10-16 12:26:16', NULL),
	(65, 'agent1', '2014-10-16 12:27:13', NULL),
	(66, 'agent1', '2014-10-16 12:37:04', NULL),
	(67, 'agent2', '2014-10-16 12:37:41', '2014-10-16 12:40:45'),
	(68, 'spv1', '2014-10-16 12:38:00', '2014-10-16 12:40:31'),
	(69, 'manager1', '2014-10-16 12:38:14', '2014-10-16 12:40:07'),
	(70, 'spv1', '2014-10-16 12:44:06', '2014-10-16 12:44:20'),
	(71, 'manager1', '2014-10-16 12:44:36', NULL),
	(72, 'spv1', '2014-10-16 12:44:50', NULL),
	(73, 'agent1', '2014-10-16 12:45:07', NULL),
	(74, 'agent2', '2014-10-16 12:45:26', NULL),
	(75, 'manager1', '2014-10-16 13:55:49', NULL),
	(76, 'spv1', '2014-10-16 13:56:04', NULL),
	(77, 'manager1', '2014-10-16 14:01:54', '2014-10-16 14:03:39'),
	(78, 'spv1', '2014-10-16 14:02:07', '2014-10-16 14:36:31'),
	(79, 'agent1', '2014-10-16 14:02:32', '2014-10-16 14:21:37'),
	(80, 'agent2', '2014-10-16 14:02:47', '2014-10-16 14:21:22'),
	(81, 'manager1', '2014-10-16 14:04:01', '2014-10-16 14:36:42');
/*!40000 ALTER TABLE `user_session_log` ENABLE KEYS */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
