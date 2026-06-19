import { createRoot } from 'react-dom/client';

export default function SwiqHome() {
    return (
        <div>
            <nav style={{
                backgroundColor: 'lightgrey',
                position: 'relative',
                top: 0,
                left: 0,
                width: '100%',
                height: '30px',
                padding: '10px',
                borderRadius: '999px',
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                textAlign: 'center',
                fontFamily: 'arial'
            }}>
                Swiq
            </nav>
        </div>
    )
}

const container = document.getElementById('root');
if (container) {
    const root = createRoot(container);
    root.render(<SwiqHome />);
}
