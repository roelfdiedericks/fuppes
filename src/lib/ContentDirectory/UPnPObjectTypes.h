
#ifndef _OBJECTTYPES_H
#define _OBJECTTYPES_H

#include <string>

typedef enum tagOBJECT_TYPE
{
  OBJECT_TYPE_UNKNOWN = 0,  
  //ITEM = 1,
  
  
  CONTAINER_STORAGE_FOLDER = 1,
  
  CONTAINER_PERSON = 10,
    CONTAINER_PERSON_MUSIC_ARTIST = 11,
    
  /* "object.container.playlistContainer" and "object.item.playlistItem”
     have the same OBJECT_TYPE in the database though the type of representation
     is selected from the configuration at runtime */
  CONTAINER_PLAYLIST_CONTAINER = 20,
  
  CONTAINER_ALBUM = 30,
    CONTAINER_ALBUM_MUSIC_ALBUM = 31,
    CONTAINER_ALBUM_PHOTO_ALBUM = 32,
    
  CONTAINER_GENRE = 40,
    CONTAINER_GENRE_MUSIC_GENRE = 41,
    CONTAINER_GENRE_MOVIE_GENRE = 42,
    
  /*CONTAINER_CHANNEL_GROUP = 50,
    CONTAINER_CHANNEL_GROUP_AUDIO_CHANNEL_GROUP = 51,
    CONTAINER_CHANNEL_GROUP_VIDEO_CHANNEL_GROUP = 52,*/
  
  //CONTAINER_EPG_CONTAINER = 60,
  
  /*CONTAINER_STORAGE_SYSTEM = 70,
  CONTAINER_STORAGE_VOLUME = 80,
  CONTAINER_BOOKMARK_FOLDER = 90*/  

  CONTAINER_MAX = 90,  



  /* we use the space between CONTAINER_MAX and ITEM for some special purpose values */
  
  
  /*
   when parsing a playlist and we find a stream url
   we set the type to "unknown broadcast" and try to 
   detect the type (audio or video) of the stream later.
   this is no valid upnp object type!
  */
  ITEM_UNKNOWN_BROADCAST = 91,



  
  ITEM = 100,  
  
  ITEM_IMAGE_ITEM = 110,
    ITEM_IMAGE_ITEM_PHOTO = 111,
	ITEM_IMAGE_ITEM_MAX = 112,
	
  ITEM_AUDIO_ITEM = 120,
    ITEM_AUDIO_ITEM_MUSIC_TRACK     = 121,
    ITEM_AUDIO_ITEM_AUDIO_BROADCAST = 122,
    //ITEM_AUDIO_ITEM_AUDIO_BOOK      = 123,
	ITEM_AUDIO_ITEM_MAX = 123,
  
  ITEM_VIDEO_ITEM = 130,
    ITEM_VIDEO_ITEM_MOVIE            = 131,
    ITEM_VIDEO_ITEM_VIDEO_BROADCAST  = 132,
    ITEM_VIDEO_ITEM_MUSIC_VIDEO_CLIP = 133,
	ITEM_VIDEO_ITEM_MAX = 133,

  ITEM_TEXT_ITEM = 140,
  /*ITEM_BOOKMARK_ITEM = 150,
  
  ITEM_EPG_ITEM = 160,
    ITEM_EPG_ITEM_AUDIO_PROGRAM = 161,
    ITEM_EPG_ITEM_VIDEO_PROGRAM = 162,*/     

  ITEM_MAX = 200
  
}OBJECT_TYPE;

#endif // _OBJECTTYPES_H
