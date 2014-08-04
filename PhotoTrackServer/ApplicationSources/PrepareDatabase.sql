CREATE TABLE Sites(
       uuid TEXT PRIMARY KEY,
       pitNumber TEXT,
       name TEXT,
       secondsSinceEpoch INTEGER,
       latitude REAL,
       longitude REAL,
       address TEXT,
       status INTEGER
       );

CREATE TABLE Photos(
       uuid TEXT PRIMARY KEY,
       imageUuid TEXT,
       imageMime TEXT,
       latitude REAL,
       longitude REAL,
       secondsSinceEpoch INTEGER,
       tag TEXT,
       siteUuid TEXT REFERENCES Sites(uuid) ON DELETE CASCADE
       );

CREATE TABLE Users(
       uuid TEXT PRIMARY KEY,
       username TEXT,
       password TEXT,   -- Hashed
       fullName TEXT,
       email TEXT,
       isSupervisor INT,
       isAdmin INT,
       organization TEXT
       );

CREATE TABLE UserSiteMap(
       user TEXT REFERENCES Users(uuid) ON DELETE CASCADE,
       site TEXT REFERENCES Sites(uuid) ON DELETE CASCADE
       );

CREATE TABLE Changes(
       seq INTEGER PRIMARY KEY AUTOINCREMENT,
       changeType INTEGER,
       photoUuid TEXT REFERENCES Photos(uuid) ON DELETE CASCADE,
       date TEXT
       );

-- Trigger to remove image after a photo is deleted
CREATE TRIGGER PhotoDeleted
AFTER DELETE ON Photos
BEGIN
  SELECT SignalImageDeleted(old.imageUuid);
END;


-- TODO Indexes
