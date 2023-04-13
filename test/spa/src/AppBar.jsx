import { useState, useRef } from 'react'
import AppBar from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import Toolbar from '@mui/material/Toolbar';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import StoreIcon from '@mui/icons-material/Store';
import IconButton from '@mui/material/IconButton';
import UploadIcon from '@mui/icons-material/Upload';
import { useAtom } from 'jotai'
import { uploadAtom, uploadCountAtom } from './atom'
import SearchBar from './SearchBar'
import _ from 'lodash'
import util from "./util";
export default function IconAppBar() {
  const inputFileRef = useRef( null );
  const [ upload, setUpload ] = useAtom(uploadAtom)
  const [ count, setCount ] = useAtom(uploadCountAtom)
  let url = '/upload';
  if( import.meta.env.DEV ) {
    url = 'http://192.168.0.60:8888/upload';
    // console.log(`[IconAppBar] app is running in development mode`)
  } else {
    // console.log(`[IconAppBar] app is running in production mode`)
  }
    
  function processFile(e){
    if (e.target.files.length == 0) return;
    setCount(e.target.files.length)
    // console.log(`111 : e.target.files.length=${e.target.files.length}`)
    // console.log(`111 : count=${count}`)
    _.each(e.target.files, f => {
      // console.log('in _.each, f=', f)
      setUpload(o=>{return {...o,[f.name]:{ progress: "0%", size: f.size, name: f.name }}});
      util.upload_file(f, (f, percent)=>{
        // console.log(`${f.name}:${percent}%`)
        setUpload(o=>{
          if( !(o && o[f.name]) ) return;
          o[f.name].progress = `${percent}%`
          return {...o}
        });
      }, f=>{
        setCount(cnt=> cnt - 1)
        // if(count == 0) setUpload({})
      }, url);
    });
    e.target.files = null
  }
  return (
    <Box sx={{ flexGrow: 1 }}>
      <AppBar position="fixed">
        <Toolbar>
          <StoreIcon sx={{ fontSize: 43, pr : 1.7 }} />
          <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
            Files
          </Typography>
          <SearchBar/>
          <IconButton
            size="large"
            edge="start"
            color="inherit"
            aria-label="menu"
            // sx={{ mr: 1.5 }}
            onClick={()=>{
              inputFileRef.current.click();
            }}
          >
            <UploadIcon />
          </IconButton>
          {/*  accept="image/*" */}
          <input type="file" multiple
            ref={inputFileRef} 
            onChange={processFile}
            hidden
          />
        </Toolbar>
      </AppBar>
    </Box>
  );
}