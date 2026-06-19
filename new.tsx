import { createRoot } from 'react-dom/client';

export default function SwiqHome() {
    return (
        <p>hi</p>
    )
}

const container = document.getElementById('root');
if (container) {
    const root = createRoot(container);
    root.render(<SwiqHome />);
}
