SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;


DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `subordinates`(IN superior SMALLINT)
BEGIN

	DECLARE not_found INT DEFAULT 0;
	DECLARE parent_id SMALLINT DEFAULT NULL;

	DECLARE subordinate_cursor CURSOR FOR SELECT subordinate_id FROM users_hierarchy WHERE superior_id = superior;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = 1;
	OPEN subordinate_cursor;

	SELECT subordinate_id FROM users_hierarchy WHERE superior_id = superior;

	REPEAT
		FETCH subordinate_cursor INTO parent_id;

		IF parent_id IS NOT NULL THEN
			CALL subordinates(parent_id);
		END IF;
	UNTIL not_found END REPEAT;

END$$

DELIMITER ;

CREATE TABLE IF NOT EXISTS `user` (
  `id` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(25) NOT NULL,
  `password` varchar(32) NOT NULL,
  `fullname` varchar(50) DEFAULT NULL,
  `level` enum('Agent','Supervisor','Manager') NOT NULL DEFAULT 'Agent',
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=4 ;

INSERT INTO `user` (`id`, `username`, `password`, `fullname`, `level`) VALUES
(1, 'agent1', '6dfb015b52c29bbcaf9dc3f1d8f1a7be', 'Test Agent Satu', 'Agent'),
(2, 'spv1', 'c44303d24fc25a450c753a64371cf1d1', 'Test Supervisor Satu', 'Supervisor'),
(3, 'manager1', 'b2e39a830f1474354e987c6820fa98ac', 'Test Manager Satu', 'Manager');

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
