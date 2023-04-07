#pragma once
#include "util.hpp"
class UploadHandler
{
private:

    fs::path upload_dir_;
    std::map<std::string, std::shared_ptr<std::ofstream>> writers_;
    std::function<void()> cb_;
    std::mutex writers_mutex_;
public:
    UploadHandler(fs::path dir, std::function<void()> cb)
    :upload_dir_( std::move(dir) ), cb_(std::move(cb))
    {
        if( !fs::exists(upload_dir_) )
        {
            fs::create_directory(upload_dir_);
        }
    }
    int write(std::string data)
    {
        try
        {
            const std::string file_name{ data.c_str() };
            auto flag = static_cast<int>(data[data.length() - 1]);
            auto buff_len = data.length() - file_name.length() -2;
            const std::string& buff = data.substr(file_name.length()+1, buff_len);
            std::string path = (upload_dir_ / file_name).string();
            std::shared_ptr<std::ofstream> writer;
            std::scoped_lock<std::mutex> lk(writers_mutex_);
            switch(flag)
            {
                case 0:
                    writer = writers_[file_name] = std::make_shared<std::ofstream>(path, std::ofstream::binary);
                    break;
                case 1:
                    writer = writers_[file_name];
                    break;
                case 2:
                    auto it = writers_.find(file_name);
                    if( it != writers_.end() )
                    {   
                        writer = it->second;
                        writers_.erase(it);
                    }
                    else 
                    {
                        writer = std::make_shared<std::ofstream>(path, std::ofstream::binary);
                    }
                    if(cb_) cb_();
                    break;
            }
            writer->write( buff.c_str(), buff.length() );
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return -1;
        }
        return 0;
    }
};


