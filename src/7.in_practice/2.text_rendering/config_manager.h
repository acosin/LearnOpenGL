#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

namespace tn
{
namespace stitching
{

class ConfigManager
{
    static ConfigManager* instance_ ;
public:
static ConfigManager* Instance() {
        if (nullptr == instance_)
            instance_ = new ConfigManager();
        return instance_;
    }

    std::string GetDefautFont() const{return "../../src/7.in_practice/2.text_rendering/QimiaoType-Regular.ttf";}
    std::string ShaderPath() { return "../../src/7.in_practice/2.text_rendering/"; }
    
};



} // namespace stitching {
} // namespace tn {
#endif  //CONFIG_MANAGER_H