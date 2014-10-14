-- --------------------------------------------------------
-- Host:                         192.168.1.8
-- Server version:               5.5.38-0ubuntu0.14.04.1 - (Ubuntu)
-- Server OS:                    debian-linux-gnu
-- HeidiSQL Version:             8.3.0.4694
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for table centini.group
CREATE TABLE IF NOT EXISTS `group` (
  `group` char(16) NOT NULL,
  `description` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`group`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.group: ~0 rows (approximately)
/*!40000 ALTER TABLE `group` DISABLE KEYS */;
/*!40000 ALTER TABLE `group` ENABLE KEYS */;


-- Dumping structure for table centini.user
CREATE TABLE IF NOT EXISTS `user` (
  `username` char(16) NOT NULL,
  `password` varchar(32) NOT NULL,
  `fullname` varchar(50) DEFAULT NULL,
  `level` enum('Agent','Supervisor','Manager') NOT NULL DEFAULT 'Agent',
  `peer` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user: ~3 rows (approximately)
/*!40000 ALTER TABLE `user` DISABLE KEYS */;
INSERT INTO `user` (`username`, `password`, `fullname`, `level`, `peer`) VALUES
	('agent1', '6dfb015b52c29bbcaf9dc3f1d8f1a7be', 'Test Agent Satu', 'Agent', NULL),
	('manager1', 'b2e39a830f1474354e987c6820fa98ac', 'Test Manager Satu', 'Manager', NULL),
	('spv1', 'c44303d24fc25a450c753a64371cf1d1', 'Test Supervisor Satu', 'Supervisor', NULL);
/*!40000 ALTER TABLE `user` ENABLE KEYS */;


-- Dumping structure for table centini.user_group
CREATE TABLE IF NOT EXISTS `user_group` (
  `id` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `username` char(16) NOT NULL,
  `group` char(16) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `user_group_fk2` (`group`),
  KEY `user_group_fk1` (`username`),
  CONSTRAINT `user_group_fk1` FOREIGN KEY (`username`) REFERENCES `user` (`username`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `user_group_fk2` FOREIGN KEY (`group`) REFERENCES `group` (`group`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user_group: ~0 rows (approximately)
/*!40000 ALTER TABLE `user_group` DISABLE KEYS */;
/*!40000 ALTER TABLE `user_group` ENABLE KEYS */;


-- Dumping structure for table centini.user_pause_log
CREATE TABLE IF NOT EXISTS `user_pause_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` char(16) DEFAULT NULL,
  `start` datetime NOT NULL,
  `finish` datetime DEFAULT NULL,
  `reason` varchar(50) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user_pause_log: ~0 rows (approximately)
/*!40000 ALTER TABLE `user_pause_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `user_pause_log` ENABLE KEYS */;


-- Dumping structure for table centini.user_session_log
CREATE TABLE IF NOT EXISTS `user_session_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` char(16) DEFAULT NULL,
  `start` datetime NOT NULL,
  `finish` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `user_session_log_fk1` (`username`),
  CONSTRAINT `user_session_log_fk1` FOREIGN KEY (`username`) REFERENCES `user` (`username`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=utf8;

-- Dumping data for table centini.user_session_log: ~24 rows (approximately)
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
	(24, 'agent1', '2014-10-14 15:32:11', NULL);
/*!40000 ALTER TABLE `user_session_log` ENABLE KEYS */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
