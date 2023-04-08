import React from 'react'
import { useAtom } from 'jotai'
import { fileAtom, filterAtom } from './atom'
import Box from '@mui/material/Box';
import Paper from '@mui/material/Paper';
import { styled } from '@mui/material/styles';
import { Download, Delete } from '@mui/icons-material';
import util from "./util";
const Item = styled(Paper)(({ theme }) => ({
  backgroundColor: theme.palette.mode === 'dark' ? '#1A2027' : '#fff',
  ...theme.typography.body2,
  padding: theme.spacing(1),
  textAlign: 'center',
  overflowWrap: 'break-word',
  color: theme.palette.text.secondary,
}));

export default function Files() {
    const [ files, setFile ] = useAtom(fileAtom)
    const [ filterTxt, setFilterTxt ] = useAtom(filterAtom)
    const listItems = files
        .filter(fi=>filterTxt? fi.name.includes(filterTxt) && fi : fi)
        .map( fi =>
        <React.Fragment key={fi.name}>
            <Box sx={{ 
                width: '100%', 
                display: 'flex', flexWrap: 'wrap', alignItems: 'center',
                border: '.2rem groove' }}>
                <Box sx={{ width: '60%', overflowWrap: 'break-word' }}>{fi.name}</Box>
                <Box sx={{ width: '20%', overflowWrap: 'break-word' }}>{util.formatFileSize(fi.size)}</Box>
                <Box sx={{ 
                    width: '20%',
                    display: 'flex', 
                    justifyContent: 'flex-end'
                    }}>
                    <a href={import.meta.env.DEV? 
                        `http://192.168.0.60:8888/store/${fi.name}` 
                        :`/store/${fi.name}`} download={fi.name}
                        target="_blank" rel="noopener noreferrer"><Download sx={{ mr: 1 }}/></a>
                    <Delete sx={{ mr: 1 }} onClick={()=>{
                        const msg = `Are you sure to delete:\n${fi.name}`
                        if (confirm(msg) == true) {
                            const url = import.meta.env.DEV? 
                            `http://192.168.0.60:8888/del` 
                            :`/del`
                            util.post_data(url, fi.path);
                        } else {

                        }
                    }}/>
                </Box>
            </Box>
        </React.Fragment>
    );
    return (
        <Box sx={{ width: '100%'}}>
            {listItems}
        </Box>
    );
}
