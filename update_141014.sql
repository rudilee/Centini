/* 7:55:19 PM Local MariaDB */ ALTER TABLE `user_pause_log` ADD CONSTRAINT `user_pause_log_fk1` FOREIGN KEY (`username`) REFERENCES `user` (`username`) ON DELETE SET NULL ON UPDATE CASCADE;
