#include "RSDK/Core/RetroEngine.hpp"

#include "CacheFiles.cpp"

#if RETRO_PLATFORM == RETRO_3DS
#include <stdio.h>
#endif

using namespace RSDK;

uint8_t RSDK::cachesEnabled = 0;

uint8_t* RSDK::staticVarCache = NULL;
uint8_t* RSDK::spriteBinCache = NULL;
uint8_t* RSDK::spriteCache = NULL;

StaticVarRef RSDK::staticVarRefs[STATIC_VAR_FILE_COUNT];
SpriteRef    RSDK::spriteBinRefs[SPRITEBIN_FILE_COUNT];

void RSDK::InitCache(uint8_t cacheFlags) {
  char pathBuf[0x40];
  uint8_t* ptr;
  FILE* fCache;
  int32 ptrOffset;

  if (cacheFlags & ENABLE_STATIC_CACHE) {
    // allocate cache
    staticVarCache = (uint8_t*) malloc(STATIC_VAR_CACHE_SIZE * sizeof(uint8_t));
    if (staticVarCache) {
      ptr = staticVarCache;

      // read data from decrypted cached file
#if RETRO_PLATFORM == RETRO_3DS
      // Save the cache in the save archive. Load the whole file using 3DS functions once at startup and process it
      // below through a memory file.
      uint32 staticCacheFileSize =
          (4 + 2 * sizeof(int32)) * STATIC_VAR_FILE_COUNT + STATIC_VAR_CACHE_SIZE;
      uint8_t *staticCacheFileData = (uint8_t *)malloc(staticCacheFileSize);

      bool32 staticCacheLoaded =
          staticCacheFileData && SKU::LoadUserFile("staticcache.bin", staticCacheFileData, staticCacheFileSize);

      if (staticCacheLoaded) {
        // sanity-check the record table: every ref must land inside the payload
        // region, otherwise treat the file as corrupt and rebuild from Data.
        for (int i = 0; i < STATIC_VAR_FILE_COUNT; i++) {
          uint8_t *rec = &staticCacheFileData[i * (4 + 2 * sizeof(int32))];
          int32 refOffset = *(int32 *)(rec + 4);
          int32 refSize   = *(int32 *)(rec + 8);

          if (refOffset < 0 || refSize < 0 || refOffset > STATIC_VAR_CACHE_SIZE - refSize) {
            staticCacheLoaded = false;
            break;
          }
        }
      }

      if (staticCacheLoaded)
        fCache = fmemopen(staticCacheFileData, staticCacheFileSize, "rb");
      else {
        if (staticCacheFileData)
          free(staticCacheFileData);
        fCache = NULL;
      }
#else
      sprintf_s(pathBuf, sizeof(pathBuf), "%sstaticcache.bin", SKU::userFileDir);
      fCache = fOpen(pathBuf, "rb");
#endif
      if (fCache) {
        for (int i = 0; i < STATIC_VAR_FILE_COUNT; i++) {
          fRead(staticVarRefs[i].hash, sizeof(uint8_t), 4, fCache);
          fRead(&ptrOffset, sizeof(int32), 1, fCache);
          fRead(&staticVarRefs[i].size, sizeof(int32), 1, fCache);

          staticVarRefs[i].ref = staticVarCache + ptrOffset;
        }

        fRead(staticVarCache, sizeof(uint8_t), STATIC_VAR_CACHE_SIZE, fCache);
        fClose(fCache);
#if RETRO_PLATFORM == RETRO_3DS
        free(staticCacheFileData);
        staticCacheFileData = NULL;
#endif
      } else {
        // if not, read from Data file/folder and save whole thing to user directory
#if RETRO_PLATFORM == RETRO_3DS
        // Save the cache in the save archive. Load the whole file using 3DS unctions once at startup and process it
        // below through a memory file.
        char *staticCacheBlob      = NULL;
        size_t staticCacheBlobSize = 0;

        fCache = open_memstream(&staticCacheBlob, &staticCacheBlobSize);
#else
        fCache = fOpen(pathBuf, "wb+");
#endif

        FileInfo info;
        InitFileInfo(&info);
        for (int i = 0; i < STATIC_VAR_FILE_COUNT; i++) {
          sprintf_s(pathBuf, sizeof(pathBuf), "Data/Objects/Static/%s", staticVarFilenames[i]);
          if (LoadFile(&info, pathBuf, FMODE_RB)) {
            // four characters *should* be enough for a hash comparison
            staticVarRefs[i].hash[0] = staticVarFilenames[i][0];
            staticVarRefs[i].hash[1] = staticVarFilenames[i][1];
            staticVarRefs[i].hash[2] = staticVarFilenames[i][2];
            staticVarRefs[i].hash[3] = staticVarFilenames[i][3];

            staticVarRefs[i].ref = ptr;
            staticVarRefs[i].size = info.fileSize;

            ptrOffset = (int32) (ptr - staticVarCache);

            if (fCache) {
              fWrite(staticVarRefs[i].hash, sizeof(uint8_t), 4, fCache);
              fWrite(&ptrOffset, sizeof(int32), 1, fCache);
              fWrite(&staticVarRefs[i].size, sizeof(int32), 1, fCache);
            }

            while (info.readPos < info.fileSize) {
              *ptr = ReadInt8(&info);
              ptr++;
            }

            CloseFile(&info);
          }
        }

        if (fCache) {
          fWrite((void*) staticVarCache, sizeof(uint8_t), STATIC_VAR_CACHE_SIZE, fCache);
          fClose(fCache);
#if RETRO_PLATFORM == RETRO_3DS
          // Persist the freshly rebuilt blob to the save archive.
          SKU::SaveUserFile("staticcache.bin", staticCacheBlob, (uint32)staticCacheBlobSize);
          free(staticCacheBlob);
#endif
        }
      }

      // set flag
      cachesEnabled |= ENABLE_STATIC_CACHE; 
    } else {
      PrintLog(PRINT_NORMAL, "Failed to allocate cache for static variables");
    }
  }

  if (cacheFlags & ENABLE_SPRITEBIN_CACHE) {
    spriteBinCache = (uint8_t*) malloc(SPRITEBIN_CACHE_SIZE * sizeof(uint8_t));
    if (spriteBinCache) {
      // only allocate sprite cache if spriteBinCache active
      if (cacheFlags & ENABLE_SPRITE_CACHE) {
        spriteCache = (uint8_t*) malloc(SPRITE_CACHE_SIZE * sizeof(uint8_t));
        if (spriteCache)
          cachesEnabled |= ENABLE_SPRITE_CACHE;
        else 
          PrintLog(PRINT_NORMAL, "Failed to allocate cache for sprites");
      }

#if RETRO_PLATFORM == RETRO_3DS
      // Save the cache in the save archive. Load the whole file using 3DS functions once at startup and process it
      // below through a memory file.
      uint32 sbinCacheFileSize = sizeof(int32) * SPRITEBIN_FILE_COUNT + SPRITEBIN_CACHE_SIZE;
      uint8_t *sbinCacheFileData = (uint8_t *)malloc(sbinCacheFileSize);

      bool32 sbinCacheLoaded =
          sbinCacheFileData && SKU::LoadUserFile("sbincache.bin", sbinCacheFileData, sbinCacheFileSize);

      if (sbinCacheLoaded) {
        // sanity-check the record table: ref offsets must stay within the payload
        for (int i = 0; i < SPRITEBIN_FILE_COUNT; i++) {
          int32 refOffset = *(int32 *)(sbinCacheFileData + i * sizeof(int32));

          if (refOffset < 0 || refOffset > SPRITEBIN_CACHE_SIZE) {
            sbinCacheLoaded = false;
            break;
          }
        }
      }

      if (sbinCacheLoaded)
        fCache = fmemopen(sbinCacheFileData, sbinCacheFileSize, "rb");
      else {
        if (sbinCacheFileData)
          free(sbinCacheFileData);
        fCache = NULL;
      }
#else
      sprintf_s(pathBuf, sizeof(pathBuf), "%ssbincache.bin", SKU::userFileDir);
      fCache = fOpen(pathBuf, "rb");
#endif

      if (fCache) {
        for (int i = 0; i < SPRITEBIN_FILE_COUNT; i++) {
          GEN_HASH_MD5(spriteBinFilenames[i], spriteBinRefs[i].hash);
          fRead(&ptrOffset, sizeof(int32), 1, fCache);

          spriteBinRefs[i].binRef = spriteBinCache + ptrOffset;
        }

        fRead(spriteBinCache, sizeof(uint8_t), SPRITEBIN_CACHE_SIZE, fCache);
        fClose(fCache);
#if RETRO_PLATFORM == RETRO_3DS
        free(sbinCacheFileData);
        sbinCacheFileData = NULL;
#endif
      } else {
#if RETRO_PLATFORM == RETRO_3DS
        // Stage the whole cache file in memory then flush it into the save archive.
        char *sbinCacheBlob      = NULL;
        size_t sbinCacheBlobSize = 0;

        fCache = open_memstream(&sbinCacheBlob, &sbinCacheBlobSize);
#else
        fCache = fOpen(pathBuf, "wb+");
#endif

        char* strPtr;
        int sprSheetRef = 0;

        FileInfo binInfo, sprInfo;
        InitFileInfo(&binInfo);
        InitFileInfo(&sprInfo);

        ptr = spriteBinCache;

        for (int i = 0; i < SPRITEBIN_FILE_COUNT; i++) {
          sprintf_s(pathBuf, sizeof(pathBuf), "Data/Sprites/%s", spriteBinFilenames[i]);
          if (LoadFile(&binInfo, pathBuf, FMODE_RB)) {
            spriteBinRefs[i].binRef = ptr;
            GEN_HASH_MD5(spriteBinFilenames[i], spriteBinRefs[i].hash);

            ptrOffset = (int32) (ptr - spriteBinCache);

            if (fCache)
              fWrite(&ptrOffset, sizeof(int32), 1, fCache);

            while (binInfo.readPos < binInfo.fileSize) {
              *ptr = ReadInt8(&binInfo);
              ptr++;
            }

            CloseFile(&binInfo);
          }
        }

        if (fCache) {
          fWrite((void*) spriteBinCache, sizeof(uint8_t), SPRITEBIN_CACHE_SIZE, fCache);
          fClose(fCache);
#if RETRO_PLATFORM == RETRO_3DS
          // Persist cache to the save archive.
          SKU::SaveUserFile("sbincache.bin", sbinCacheBlob, (uint32)sbinCacheBlobSize);
          free(sbinCacheBlob);
#endif
        }
      }

    } else {
      PrintLog(PRINT_NORMAL, "Failed to allocate cache for sprite bins");
    }

    cachesEnabled |= ENABLE_SPRITEBIN_CACHE;
  }

  if (spriteBinCache) {
    // TODO: cache all GIF files
  }
}

void RSDK::FreeCache() {
  if (staticVarCache)
    free(staticVarCache);

  if (spriteBinCache)
    free(spriteBinCache);

  if (spriteCache)
    free(spriteCache);
}
