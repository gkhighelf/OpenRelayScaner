/**
 * Created by GKHighElf - gkhighelf@gmail.com
 * in near 2000 year, for self education and use.
**/

create database statistics;

DROP TABLE IF EXISTS `smtps`;
CREATE TABLE `smtps` (
  `subnet` int(4) unsigned NOT NULL default '0',
  `mask` int(4) unsigned NOT NULL default '0',
  `ip` int(4) unsigned NOT NULL default '0',
  `r1` char(255) NOT NULL default '',
  `r2` char(255) NOT NULL default '',
  `r3` char(255) NOT NULL default '',
  `r4` char(255) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `world_subnets`;
CREATE TABLE `world_subnets` (
  `subnet` int(4) unsigned NOT NULL default '0',
  `mask` int(4) unsigned NOT NULL default '0',
  `country` char(2) NOT NULL default 'UA',
  `s_subnet` char(15) NOT NULL default '',
  `assigned` int(1) unsigned NOT NULL default '0',
  `iip` int(1) unsigned NOT NULL default '0',
  `pos` int(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`subnet`),
  KEY `iip` (`iip`),
  KEY `assigned` (`assigned`,`iip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;