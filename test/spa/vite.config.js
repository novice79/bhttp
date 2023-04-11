import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vitejs.dev/config/
export default defineConfig({
  server: {
    host: true
  },
  plugins: [react()],
  build: {
    outDir: `../../dist/${process.env.OSTYPE}/bin/www`,
    emptyOutDir: true
  }
})
