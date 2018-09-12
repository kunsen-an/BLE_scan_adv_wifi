#ifndef __HASHMAP_H__
#define __HASHMAP_H__ 1

#include <unordered_map>

class MapElement
{
  public:
    int id;    // id number
    int round; // round
    int rssi;  // received signal strength indicator
  private:
    static int idNumber;

  public:
    MapElement()
    {
        this->id = ++idNumber;
    }

    static int getIdNumber()
    {
        return idNumber;
    }

    virtual const char *type()
    {
        return "X";
    }
};

class WiFiMapElement : public MapElement
{
  public:
    using MapElement::MapElement;

    const char *type()
    {
        return "WiFi";
    }
};

class BLEMapElement : public MapElement
{
  public:
    using MapElement::MapElement;

    const char *type()
    {
        return "BLE";
    }
};

typedef std::unordered_map<std::string, MapElement *> IdMap;

#endif // __HASHMAP_H__
